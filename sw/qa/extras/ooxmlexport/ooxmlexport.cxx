/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <o3tl/cppunittraitshelper.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/UnitConversion.hxx>
#include <frameformats.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr) {}
};

CPPUNIT_TEST_FIXTURE(Test, testfdo81381)
{
    loadAndSave("fdo81381.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:object[1]/o:OLEObject[1]"_ostr, "DrawAspect"_ostr, u"Icon"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtAlias)
{
    loadAndSave("sdt-alias.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // <w:alias> was completely missing.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:alias"_ostr, "val"_ostr, u"Subtitle"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFooterBodyDistance)
{
    loadAndSave("footer-body-distance.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Page break was exported as section break, this was 0
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:br"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testfdo81031, "fdo81031.docx")
{
    // vml image was not rendered
    // As there are also numPicBullets in the file,
    // the fragmentPath was not changed hence relationships were not resolved.

    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> xImage(image, uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic = getProperty<uno::Reference<graphic::XGraphic> >(xImage, u"Graphic"_ustr);
    uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(381), xBitmap->getSize().Width );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(148), xBitmap->getSize().Height );
}

CPPUNIT_TEST_FIXTURE(Test, testPlausableBorder)
{
    loadAndSave("plausable-border.docx");
    // sw::util::IsPlausableSingleWordSection() did not merge two page styles due to borders.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Page break was exported as section break, this was 0
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:br"_ostr, 1);

    CPPUNIT_ASSERT_EQUAL( 2, getPages() );
}

CPPUNIT_TEST_FIXTURE(Test, testUnwantedSectionBreak)
{
    loadAndSave("unwanted-section-break.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 2: an additional sectPr was added to the document.
    assertXPath(pXmlDoc, "//w:sectPr"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testfdo80897 )
{
    loadAndSave("fdo80897.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:bodyPr/a:prstTxWarp"_ostr, "prst"_ostr, u"textTriangle"_ustr);
}


DECLARE_OOXMLEXPORT_TEST(testFdo80997, "fdo80997.docx")
{
    // The problem was that the DOCX exporter not able to export text behind textbox, if textbox has a wrap property.
    uno::Reference< text::XTextRange > xParagraph = getParagraph( 1 );
    getRun( xParagraph, 1, u" text"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo80902)
{
    loadAndSave("fdo80902.docx");
    // The problem was that the docGrid type was set as default so fix it for other grid type
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:docGrid"_ostr, "type"_ostr, u"lines"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testParaShading)
{
    loadAndSave("para-shading.docx");
    // Make sure the themeColor attribute is not written when it would be empty.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:shd"_ostr, "themeColor"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO83044)
{
    loadAndSave("fdo83044.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:text"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testfdo83428, "fdo83428.docx")
{
     uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
     uno::Reference<document::XDocumentProperties> xProps(xDocumentPropertiesSupplier->getDocumentProperties());
     uno::Reference<beans::XPropertySet> xUDProps(xProps->getUserDefinedProperties(), uno::UNO_QUERY);
     CPPUNIT_ASSERT_EQUAL(u"Document"_ustr, getProperty<OUString>(xUDProps, u"Testing"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testShapeInFloattable)
{
    loadAndSave("shape-in-floattable.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // No nested drawingML w:txbxContent.
    assertXPath(pXmlDoc, "//mc:Choice//w:txbxContent//w:txbxContent"_ostr, 0);
    // Instead, make sure we have a separate shape and a table
    assertXPath(pXmlDoc, "//mc:AlternateContent//mc:Choice[@Requires='wpg']"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testEmptyAnnotationMark)
{
    loadAndReload("empty-annotation-mark.docx");
    // Delete the word that is commented, and save again.
    uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 3);
    CPPUNIT_ASSERT_EQUAL(u"with"_ustr, xRun->getString());
    xRun->setString(u""_ustr);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->store();

    // Then inspect the OOXML markup of the modified document model.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // There were two commentReference nodes.
    assertXPath(pXmlDoc, "//w:commentReference"_ostr, "id"_ostr, u"0"_ustr);
    // Empty comment range was not ignored on export, this was 1.
    assertXPath(pXmlDoc, "//w:commentRangeStart"_ostr, 0);
    // Ditto.
    assertXPath(pXmlDoc, "//w:commentRangeEnd"_ostr, 0);
}

DECLARE_OOXMLEXPORT_TEST(testDropdownInCell, "dropdown-in-cell.docx")
{
    // First problem: table was missing from the document, this was 0.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // Second problem: dropdown shape wasn't anchored inside the B1 cell.
    if (getShapes() > 0)
    {
        uno::Reference<text::XTextContent> xShape(getShape(1), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xAnchor = xShape->getAnchor();
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRangeCompare> xTextRangeCompare(xCell, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextRangeCompare->compareRegionStarts(xAnchor, xCell));
    }
    else if (!isExported())
    {
        // ComboBox was imported as DropDown text field
        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
        CPPUNIT_ASSERT(xFields->hasMoreElements());
        uno::Any aField = xFields->nextElement();
        uno::Reference<lang::XServiceInfo> xServiceInfo(aField, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.textfield.DropDown"_ustr));
    }
    else
    {
        // DropDown text field is exported as inline SDT, we import that back here.
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<table::XCell> xCell = xTable->getCellByName(u"B1"_ustr);
        uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xCell, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParagraphs = xParagraphsAccess->createEnumeration();
        uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
        uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);
        OUString aPortionType;
        xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
        CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
        uno::Reference<text::XTextContent> xContentControl;
        xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
        uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValues> aListItems;
        xContentControlProps->getPropertyValue(u"ListItems"_ustr) >>= aListItems;
        CPPUNIT_ASSERT(aListItems.hasElements());
    }
}

DECLARE_OOXMLEXPORT_TEST(testTableAlignment, "table-alignment.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was LEFT_AND_WIDTH, i.e. table alignment wasn't imported correctly.
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::RIGHT, getProperty<sal_Int16>(xTable, u"HoriOrient"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testSdtIgnoredFooter)
{
    loadAndSave("sdt-ignored-footer.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 1, make sure no w:sdt sneaks into the main document from the footer.
    assertXPath(pXmlDoc, "//w:sdt"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtRunPicture)
{
    loadAndSave("sdt-run-picture.docx");
    // SDT around run was exported as SDT around paragraph
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 1: there was an SDT around w:p.
    assertXPath(pXmlDoc, "//w:body/w:sdt"_ostr, 0);
    // This was 0: there were no SDT around w:r.
    assertXPath(pXmlDoc, "//w:body/w:p/w:sdt"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testChartDupe, "chart-dupe.docx")
{
    // Single chart was exported back as two charts.
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    // This was 2, on second import we got a duplicated chart copy.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xEmbeddedObjects->getCount());


    if (!isExported())
       return; // initial import

    xmlDocUniquePtr pXmlDocCT = parseExport(u"[Content_Types].xml"_ustr);

    assertXPath(pXmlDocCT,
        "/ContentType:Types/ContentType:Override[@PartName='/word/charts/chart1.xml']"_ostr,
        "ContentType"_ostr,
        u"application/vnd.openxmlformats-officedocument.drawingml.chart+xml"_ustr);
    assertXPath(pXmlDocCT, "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/Microsoft_Excel_Worksheet1.xlsx']"_ostr, "ContentType"_ostr, u"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"_ustr);

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/charts/_rels/chart1.xml.rels"_ustr);
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='../embeddings/Microsoft_Excel_Worksheet1.xlsx']"_ostr,
        "Type"_ostr,
        u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package"_ustr);
    // check the content too
    xmlDocUniquePtr pXmlDocChart1 = parseExport(u"word/charts/chart1.xml"_ustr);
    assertXPath(pXmlDocChart1,
        "/c:chartSpace/c:externalData"_ostr,
        "id"_ostr,
        u"rId1"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testPositionAndRotation, "position-and-rotation.docx")
{
    // The document should look like: "This line is tricky, <image> because only 'This line is tricky,' is on the left."
    // But the image was pushed down, so it did not break the line into two text portions.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Should be 1559, was -5639
    CPPUNIT_ASSERT(xShape->getPosition().X > 1500);
    // Should be 88, was 473
    CPPUNIT_ASSERT(xShape->getPosition().Y < 100);
}

DECLARE_OOXMLEXPORT_TEST(testNumberingFont, "numbering-font.docx")
{
    // check that the original numrule font name is still Calibri
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"CharacterStyles"_ustr)->getByName(u"ListLabel 1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Calibri"_ustr, getProperty<OUString>(xStyle, u"CharFontName"_ustr));

    uno::Reference<text::XTextRange> xPara = getParagraph(2);
    uno::Reference<beans::XPropertySet> properties(xPara, uno::UNO_QUERY);
    uno::Any aValue = properties->getPropertyValue(u"ListAutoFormat"_ustr);
    CPPUNIT_ASSERT(aValue.hasValue());
    uno::Sequence<beans::NamedValue> aListAutoFormat;
    CPPUNIT_ASSERT(aValue >>= aListAutoFormat);
    auto it = std::find_if(std::cbegin(aListAutoFormat), std::cend(aListAutoFormat),
        [](const css::beans::NamedValue& val) { return val.Name == "CharFontName"; });
    CPPUNIT_ASSERT(it != std::cend(aListAutoFormat));
    OUString sOverrideFontName;
    CPPUNIT_ASSERT(it->Value >>= sOverrideFontName);
    // but the overridden font name is Verdana
    CPPUNIT_ASSERT_EQUAL(u"Verdana"_ustr, sOverrideFontName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf106541_noinheritChapterNumbering)
{
    loadAndReload("tdf106541_noinheritChapterNumbering.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // in LO, it appears that styles based on the Chapter Numbering style explicitly set the
    // numbering style/outline level to 0 by default, and that LO prevents inheriting directly from "Outline" style.
    // Adding this preventative unit test to ensure that any fix for tdf106541 doesn't make incorrect assumptions.

//reverting tdf#76817 hard-codes the numbering style on the paragraph, preventing RT of "Outline" style
//    CPPUNIT_ASSERT_EQUAL(OUString("Outline"), getProperty<OUString>(getParagraph(1), "NumberingStyleName"));

    OUString sPara3NumberingStyle = getProperty<OUString>(getParagraph(3), u"NumberingStyleName"_ustr);
    CPPUNIT_ASSERT_EQUAL(sPara3NumberingStyle, getProperty<OUString>(getParagraph(4), u"NumberingStyleName"_ustr));

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//body/txt/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr, 3);  //three of the four paragraphs have numbering
    assertXPath(pXmlDoc, "//body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr, "expand"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "//body/txt[2]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr, 0); //second paragraph style disables numbering
    assertXPath(pXmlDoc, "//body/txt[3]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr, "expand"_ostr, u"I."_ustr);
    assertXPath(pXmlDoc, "//body/txt[4]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr, "expand"_ostr, u"II."_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf53856_conflictingStyle, "tdf53856_conflictingStyle.docx")
{
    // The "Text" style conflicted with builtin paragraph style Caption -> Text
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Text"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Times New Roman"_ustr, getProperty<OUString>(xStyle, u"CharFontName"_ustr));
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xStyle, u"CharPosture"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf104713_undefinedStyles, "tdf104713_undefinedStyles.docx")
{
    // Normal paragraph style was not defined, so don't replace conflicting styles
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Heading 1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(212), getProperty<sal_Int32>(xStyle, u"ParaBottomMargin"_ustr));

    // tdf108765: once importing is finished, use default values for any styles not yet defined.
    xStyle.set( getStyles(u"ParagraphStyles"_ustr)->getByName(u"Footnote"_ustr), uno::UNO_QUERY );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Font size", 10.f, getProperty<float>(xStyle, u"CharHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testDrawingmlFlipv)
{
    loadAndSave("drawingml-flipv.docx");
    // The problem was that the shape had vertical flip only, but then we added rotation as well on export.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPathNoAttribute(pXmlDoc, "//a:xfrm"_ostr, "rot"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testRot90Fliph)
{
    loadAndSave("rot90-fliph.docx");
    // The problem was that a shape rotation of 90° got turned into 270° after roundtrip.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//a:xfrm"_ostr, "flipH"_ostr, u"1"_ustr);
    // This was 16200000 (270 * 60000).
    assertXPath(pXmlDoc, "//a:xfrm"_ostr, "rot"_ostr, u"5400000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testRot180Flipv)
{
    loadAndSave("rot180-flipv.docx");
    // 180° rotation got lost after roundtrip.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//a:xfrm"_ostr, "flipV"_ostr, u"1"_ustr);
    // This attribute was completely missing.
    assertXPath(pXmlDoc, "//a:xfrm"_ostr, "rot"_ostr, u"10800000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testRot270Flipv)
{
    loadAndSave("rot270-flipv.docx");
    // 270° rotation got turned into 90° after roundtrip.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//a:xfrm"_ostr, "flipV"_ostr, u"1"_ustr);
    // This was 5400000.
    assertXPath(pXmlDoc, "//a:xfrm"_ostr, "rot"_ostr, u"16200000"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testWpsCharColor, "wps-char-color.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was -1, i.e. the character color was default (-1), not white.
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<Color>(xShape->getStart(), u"CharColor"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTableStyleCellBackColor, "table-style-cell-back-color.docx")
{
    // The problem was that cell background was white, not green.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName(u"A1"_ustr);
    // This was 0xffffff.
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, getProperty<Color>(xCell, u"BackColor"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTableStyleBorder, "table-style-border.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // This was 0, the second cell was missing its right border.
    uno::Reference<table::XCell> xCell = xTable->getCellByName(u"A2"_ustr);
    CPPUNIT_ASSERT(getProperty<table::BorderLine2>(xCell, u"RightBorder"_ustr).LineWidth > 0);

    // This was also 0 (even after fixing the previous problem), the first cell was missing its right border, too.
    xCell = xTable->getCellByName(u"A1"_ustr);
    CPPUNIT_ASSERT(getProperty<table::BorderLine2>(xCell, u"RightBorder"_ustr).LineWidth > 0);
}

DECLARE_OOXMLEXPORT_TEST(testTableStyleBorderExport, "table-style-border-export.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName(u"A3"_ustr);
    // Bottom border was white, so this was 0xffffff.
    CPPUNIT_ASSERT_EQUAL(Color(0x8064A2), Color(ColorTransparency, getProperty<table::BorderLine2>(xCell, u"BottomBorder"_ustr).Color));
}

DECLARE_OOXMLEXPORT_TEST(testAnchorPosition, "anchor-position.docx")
{
    // The problem was that the at-char anchored picture was at the end of the
    // paragraph, so there were only two positions: a Text, then a Frame one.
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(getRun(getParagraph(1), 1), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr, getProperty<OUString>(getRun(getParagraph(1), 2), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(getRun(getParagraph(1), 3), u"TextPortionType"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testMultiPageToc, "multi-page-toc.docx")
{
    // Import of this document triggered an STL assertion.

    // Document has a ToC from its second paragraph.
    uno::Reference<container::XNamed> xTextSection = getProperty< uno::Reference<container::XNamed> >(getParagraph(2), u"TextSection"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Table of Contents1"_ustr, xTextSection->getName());
    // There should be a field in the header as well.
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), u"HeaderText"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"TextField"_ustr, getProperty<OUString>(getRun(getParagraphOfText(1, xHeaderText), 1), u"TextPortionType"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTextboxTable, "textbox-table.docx")
{
    // We wrote not-well-formed XML during export for this one:
    // Shape with textbox, having a table and also anchored inside a table.

    // Just make sure that we have both tables.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testCropPixel)
{
    loadAndSave("crop-pixel.docx");
    // If map mode of the graphic is in pixels, then we used to handle original
    // size of the graphic as mm100, but it was in pixels.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This is 17667 in the original document, was 504666 (so the image
    // become invisible), now is around 19072.
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "//a:srcRect"_ostr, "l"_ostr).toInt32() <= 22452);
}
/* FixMe: tdf#142805 Test disabled, because the picture is not load at all.
CPPUNIT_TEST_FIXTURE(Test, testEffectExtent)
{
    loadAndSave("effect-extent.docx");
    // The problem was that in case there were no shadows on the picture, we
    // wrote a <wp:effectExtent> full or zeros.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // E.g. this was 0.
    assertXPath(pXmlDoc, "//wp:effectExtent", "l", "114300");
}
*/
CPPUNIT_TEST_FIXTURE(Test, testEffectExtentInline)
{
    loadAndSave("effect-extent-inline.docx");
    // The problem was that in case there was inline rotated picture, we
    // wrote a <wp:effectExtent> full or zeros.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // E.g. this was 0.
    assertXPath(pXmlDoc, "//wp:effectExtent"_ostr, "l"_ostr, u"609600"_ustr);
    assertXPath(pXmlDoc, "//wp:effectExtent"_ostr, "r"_ostr, u"590550"_ustr);
    assertXPath(pXmlDoc, "//wp:effectExtent"_ostr, "t"_ostr, u"590550"_ustr);
    assertXPath(pXmlDoc, "//wp:effectExtent"_ostr, "b"_ostr, u"571500"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testEm, "em.docx")
{
    // Test all possible <w:em> arguments.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::NONE, getProperty<sal_Int16>(getRun(getParagraph(1), 1), u"CharEmphasis"_ustr));
    // This was ACCENT_ABOVE.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::DOT_ABOVE, getProperty<sal_Int16>(getRun(getParagraph(1), 2), u"CharEmphasis"_ustr));
    // This was DOT_ABOVE.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::ACCENT_ABOVE, getProperty<sal_Int16>(getRun(getParagraph(1), 3), u"CharEmphasis"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::CIRCLE_ABOVE, getProperty<sal_Int16>(getRun(getParagraph(1), 4), u"CharEmphasis"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::DOT_BELOW, getProperty<sal_Int16>(getRun(getParagraph(1), 5), u"CharEmphasis"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testFdo77716, "fdo77716.docx")
{
    // The problem was that there should be 200 twips spacing between the two paragraphs, but there wasn't any.
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    // This was 0.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(200)), getProperty<sal_Int32>(xStyle, u"ParaBottomMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testAfterlines, "afterlines.docx")
{
    // This was 353, i.e. the value of <w:spacing w:after="200"> from <w:pPrDefault>, instead of <w:spacing w:afterLines="100"/> from <w:pPr>.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(423), getProperty<sal_Int32>(getParagraph(1), u"ParaBottomMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testParagraphMark, "paragraph-mark.docx")
{
    // The problem was that we didn't handle the situation when an empty paragraph's marker had both a char style and some direct formatting.

    // This was 11.
    CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(getParagraph(1), u"CharHeight"_ustr));
    // This was empty.
    CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr, getProperty<OUString>(getRun(getParagraph(1), 1), u"CharStyleName"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testParagraphMark2, "paragraph-mark2.docx")
{
    // The problem was that we didn't handle the situation when an empty paragraph's marker had both a char style and some direct formatting.

    // This was Segoe UI, set by Char Style FontStyle11 presumably.
    CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(getRun(getParagraph(1), 1), u"CharFontName"_ustr));
    // This was 11, set by Char Style FontStyle11 presumably.
    CPPUNIT_ASSERT_EQUAL(10.f, getProperty<float>(getRun(getParagraph(1), 1), u"CharHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testParagraphMarkNonempty)
{
    loadAndSave("paragraph-mark-nonempty.odt");
    validate(maTempFile.GetFileName(), test::OOXML);
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // There were two <w:sz> elements, make sure the 40 one is dropped and the 20 one is kept.
    assertXPath(pXmlDoc, "//w:p/w:pPr/w:rPr/w:sz"_ostr, "val"_ostr, u"20"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testPageBreakBefore, "page-break-before.docx")
{
    // This was style::BreakType_PAGE_BEFORE, i.e. page break wasn't ignored, as it should have been.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE, getProperty<style::BreakType>(getParagraph(2), u"BreakType"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTableRtl, "table-rtl.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was text::WritingMode2::LR_TB, i.e. direction of the table was ignored.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>(xTable, u"WritingMode"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTableLr, "table-ltr.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was text::WritingMode2::RL_TB, i.e. direction of the table was wrongly guessed.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::CONTEXT, getProperty<sal_Int16>(xTable, u"WritingMode"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlCjklist30, "cjklist30.docx")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::TIAN_GAN_ZH, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlCjklist31, "cjklist31.docx")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::DI_ZI_ZH, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlCjklist34, "cjklist34.docx")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_UPPER_ZH_TW, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlCjklist35, "cjklist35.docx")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_LOWER_ZH, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlCjklist44, "cjklist44.docx")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_DIGITAL2_KO, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlTextNumberList, "text_number_list.docx")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::TEXT_NUMBER, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlTextCardinalList, "text_cardinal_list.docx")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::TEXT_CARDINAL, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlTextOrdinalList, "text_ordinal_list.docx")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::TEXT_ORDINAL, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlSymbolChicagoList, "symbol_chicago_list.docx")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::SYMBOL_CHICAGO, numFormat);
}

CPPUNIT_TEST_FIXTURE(Test, testOoxmlNumListZHTW)
{
    loadAndSave("numlist-zhtw.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);

    assertXPath ( pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:numFmt"_ostr,"val"_ostr,u"taiwaneseCountingThousand"_ustr );
}

CPPUNIT_TEST_FIXTURE(Test, testOoxmlNumListZHCN)
{
    loadAndSave("numlist-zhcn.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);

    assertXPath ( pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:numFmt"_ostr,"val"_ostr,u"chineseCountingThousand"_ustr );
}

CPPUNIT_TEST_FIXTURE(Test, testOOxmlOutlineNumberTypes)
{
    loadAndSave("outline-number-types.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);

    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:pStyle"_ostr, "val"_ostr, u"Heading1"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:numFmt"_ostr, "val"_ostr, u"none"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[2]/w:numFmt"_ostr, "val"_ostr, u"decimalEnclosedCircle"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[3]/w:numFmt"_ostr, "val"_ostr, u"decimal"_ustr); // CHARS_GREEK_UPPER_LETTER fallback to decimal
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[4]/w:numFmt"_ostr, "val"_ostr, u"decimal"_ustr); // CHARS_GREEK_LOWER_LETTER fallback to decimal
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[5]/w:numFmt"_ostr, "val"_ostr, u"arabicAlpha"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[6]/w:numFmt"_ostr, "val"_ostr, u"hindiVowels"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[7]/w:numFmt"_ostr, "val"_ostr, u"thaiLetters"_ustr);

    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[1]/w:numFmt"_ostr, "val"_ostr, u"decimal"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[2]/w:numFmt"_ostr, "val"_ostr, u"decimal"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[3]/w:numFmt"_ostr, "val"_ostr, u"decimal"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[4]/w:numFmt"_ostr, "val"_ostr, u"decimal"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[5]/w:numFmt"_ostr, "val"_ostr, u"russianUpper"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[6]/w:numFmt"_ostr, "val"_ostr, u"russianLower"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[7]/w:numFmt"_ostr, "val"_ostr, u"russianUpper"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[8]/w:numFmt"_ostr, "val"_ostr, u"russianLower"_ustr);

    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[3]/w:lvl[1]/w:numFmt"_ostr, "val"_ostr, u"decimal"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[3]/w:lvl[2]/w:numFmt"_ostr, "val"_ostr, u"decimal"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[3]/w:lvl[3]/w:numFmt"_ostr, "val"_ostr, u"decimal"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[3]/w:lvl[4]/w:numFmt"_ostr, "val"_ostr, u"decimal"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[3]/w:lvl[5]/w:numFmt"_ostr, "val"_ostr, u"decimal"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[3]/w:lvl[6]/w:numFmt"_ostr, "val"_ostr, u"decimal"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[3]/w:lvl[7]/w:numFmt"_ostr, "val"_ostr, u"decimal"_ustr);

}

DECLARE_OOXMLEXPORT_TEST(testNumParentStyle, "num-parent-style.docx")
{
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                        getProperty<sal_Int32>(getParagraph(1), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, getProperty<OUString>(getParagraph(1), u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                        getProperty<sal_Int32>(getParagraph(2), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"1.1"_ustr, getProperty<OUString>(getParagraph(2), u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                        getProperty<sal_Int32>(getParagraph(3), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, getProperty<OUString>(getParagraph(3), u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                        getProperty<sal_Int32>(getParagraph(4), u"OutlineLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"2.1"_ustr, getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testNumOverrideLvltext, "num-override-lvltext.docx")
{
    uno::Reference<container::XIndexAccess> xRules = getProperty< uno::Reference<container::XIndexAccess> >(getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), u"NumberingRules"_ustr);
    // This was 1, i.e. the numbering on the second level was "1", not "1.1".
    // Check the paragraph properties, not the list ones, since they can differ due to overrides
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"1.1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));

    // The paragraph marker's red font color was inherited by the number portion, this was ff0000.
    CPPUNIT_ASSERT_EQUAL(u"ffffffff"_ustr, parseDump("//SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']/SwFont"_ostr, "color"_ostr));
}

DECLARE_OOXMLEXPORT_TEST(testNumOverrideStart, "num-override-start.docx")
{
    uno::Reference<container::XIndexAccess> xRules = getProperty< uno::Reference<container::XIndexAccess> >(getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), u"NumberingRules"_ustr);
    // List starts with "1.1"
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), comphelper::SequenceAsHashMap(xRules->getByIndex(1))[u"StartWith"_ustr].get<sal_Int16>());
    // But paragraph starts with "1.3"
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
    OUString listId;
    CPPUNIT_ASSERT(xPara->getPropertyValue(u"ListId"_ustr) >>= listId);
    CPPUNIT_ASSERT_EQUAL(u"1.3"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTextboxRightEdge, "textbox-right-edge.docx")
{
    // I'm fairly sure this is not specific to DOCX, but the doc model created
    // by the ODF import doesn't trigger this bug, so let's test this here
    // instead of uiwriter.
    int nShapeLeft = parseDump("//anchored/SwAnchoredDrawObject/bounds"_ostr, "left"_ostr).toInt32();
    int nShapeWidth = parseDump("//anchored/SwAnchoredDrawObject/bounds"_ostr, "width"_ostr).toInt32();
    int nTextboxLeft = parseDump("//anchored/fly/infos/bounds"_ostr, "left"_ostr).toInt32();
    int nTextboxWidth = parseDump("//anchored/fly/infos/bounds"_ostr, "width"_ostr).toInt32();
    // This is a rectangle, make sure the right edge of the textbox is still
    // inside the draw shape.
    CPPUNIT_ASSERT(nShapeLeft + nShapeWidth >= nTextboxLeft + nTextboxWidth);
}

DECLARE_OOXMLEXPORT_TEST(testEffectExtentMargin, "effectextent-margin.docx")
{
    // This was 318, i.e. oox::drawingml::convertEmuToHmm(114300), effectExtent
    // wasn't part of the margin, leading to the fly not taking enough space
    // around itself.
    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(114300+95250), getProperty<sal_Int32>(getShape(1), u"LeftMargin"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf88583)
{
    loadAndReload("tdf88583.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(getParagraph(1), u"FillStyle"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0x00cc00), getProperty<Color>(getParagraph(1), u"FillColor"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf97090, "tdf97090.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0x95B3D7), getProperty<Color>(xTable->getCellByName(u"A1"_ustr), u"BackColor"_ustr));

    uno::Reference<container::XEnumerationAccess> paraEnumAccess(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    assert( paraEnumAccess.is() );
    uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();

    assert( paraEnum.is() );
    uno::Reference<beans::XPropertySet> paragraphProperties(paraEnum->nextElement(), uno::UNO_QUERY);
    assert( paragraphProperties.is() );
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(paragraphProperties, u"FillStyle"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<Color>(paragraphProperties, u"FillColor"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf89791, "tdf89791.docx")
{
    if (isExported())
    {
        uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
        CPPUNIT_ASSERT_EQUAL(false, bool(xNameAccess->hasByName(u"docProps/custom.xml"_ustr)));
    }

    //tdf#102619 - setting FollowStyle with a not-yet-created style was failing. (Titre is created before Corps de texte).
    uno::Reference< beans::XPropertySet > properties(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Titre"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Corps de texte"_ustr, getProperty<OUString>(properties, u"FollowStyle"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf91261, "tdf91261.docx")
{
    bool snapToGrid = true;
    uno::Reference< text::XTextRange > xPara = getParagraph( 2 );
    uno::Reference< beans::XPropertySet > properties( xPara, uno::UNO_QUERY);
    properties->getPropertyValue(u"SnapToGrid"_ustr) >>= snapToGrid ;
    CPPUNIT_ASSERT_EQUAL(false, snapToGrid);

    uno::Reference< beans::XPropertySet> xStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    sal_Int16 nGridMode;
    xStyle->getPropertyValue(u"GridMode"_ustr) >>= nGridMode;
    CPPUNIT_ASSERT_EQUAL( sal_Int16(2), nGridMode);

    bool bGridSnapToChars;
    xStyle->getPropertyValue(u"GridSnapToChars"_ustr) >>= bGridSnapToChars;
    CPPUNIT_ASSERT_EQUAL(true, bGridSnapToChars);
}

DECLARE_OOXMLEXPORT_TEST(testTdf89890, "tdf89890.docx")
{
    // Numbering picture bullet was too large.
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    bool bFound = false;
    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "GraphicSize")
        {
            // Height of the graphic was too large: 4382 after import, then 2485 after roundtrip.
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(279), rProp.Value.get<awt::Size>().Height);
            bFound = true;
        }
    }
    CPPUNIT_ASSERT(bFound);
}

DECLARE_OOXMLEXPORT_TEST(testTdf91594, "tdf91594.docx")
{
    uno::Reference<text::XTextRange> xPara1(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(u'\xf0fb', xPara1->getString()[0] );
    uno::Reference<text::XTextRange> xPara2(getParagraph(2));
    CPPUNIT_ASSERT_EQUAL(u'\xf0fc', xPara2->getString()[0] );
    uno::Reference<text::XTextRange> xPara3(getParagraph(3));
    CPPUNIT_ASSERT_EQUAL(u'\xf0fd', xPara3->getString()[0] );
    uno::Reference<text::XTextRange> xPara4(getParagraph(4));
    CPPUNIT_ASSERT_EQUAL(u'\xf0fe', xPara4->getString()[0] );

    uno::Reference<beans::XPropertySet> xRun(getRun(xPara1,1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Wingdings"_ustr, getProperty<OUString>(xRun, u"CharFontName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Wingdings"_ustr, getProperty<OUString>(xRun, u"CharFontNameAsian"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Wingdings"_ustr, getProperty<OUString>(xRun, u"CharFontNameComplex"_ustr));
}
DECLARE_OOXMLEXPORT_TEST(testTDF99434, "protectedform.docx")
{
    css::uno::Reference<css::lang::XMultiServiceFactory> m_xTextFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet > xSettings(m_xTextFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY);
    uno::Any aProtect = xSettings->getPropertyValue(u"ProtectForm"_ustr);
    bool bProt = false;
    aProtect >>= bProt;
    CPPUNIT_ASSERT(bProt);
}

DECLARE_OOXMLEXPORT_TEST(testTdf44986, "tdf44986.docx")
{
    // Check that the table at the second paragraph.
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
    // Check the first row of the table, it should have two cells (one separator).
    // This was 0: the first row had no separators, so it had only one cell, which was too wide.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), u"TableColumnSeparators"_ustr).getLength());
    // Check content of cells, including the newly added gridAfter cell
    CPPUNIT_ASSERT_EQUAL(u"A1"_ustr, uno::Reference<text::XTextRange>(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY_THROW)->getString());
    CPPUNIT_ASSERT_EQUAL(u"A2"_ustr, uno::Reference<text::XTextRange>(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY_THROW)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, uno::Reference<text::XTextRange>(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY_THROW)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf118682)
{
    loadAndSave("tdf118682.fodt");
    // Support cell references in table formulas
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Formula fields were completely missing.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r/w:fldChar"_ostr, 3);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r/w:fldChar"_ostr, 3);

    // Cell references were parenthesized: <A1>+<A2> and SUM(<A1:A3>)
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r[2]/w:instrText"_ostr, u" =A1+A2"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[2]/w:instrText"_ostr, u" =SUM(A1:A3)"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133163)
{
    loadAndSave("tdf133163.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Formula cells were completely missing.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r/w:fldChar"_ostr, 3);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r/w:fldChar"_ostr, 3);

    // Cell references were parenthesized: <A1>+<A2> and SUM(<A1:A3>)
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r[2]/w:instrText"_ostr, u" =A1+A2"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[2]/w:instrText"_ostr, u" =SUM(A1:A3)"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133647)
{
    loadAndSave("tdf133647.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Keep original formula during round-trip
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =SUM(A1,B1)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =SUM(C1:D1)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =SUM(A1,5,B1:C1,6)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[7]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =(1+2)*SUM(C1,D1)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[8]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =3*(2+SUM(A1:C1)+7)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[9]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =1+(SUM(1,2))"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[10]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =(SUM(C1,5)*(2+7))*(3+SUM(1,B1))"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[11]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =sum(a1,b1)"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123386)
{
    loadAndSave("tdf123386.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Keep original formula during round-trip
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =A1 < 2"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =B1 > 1"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =C1=3"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =D1 <> 3"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[7]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =AND(A1=1,B1=2)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[8]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =AND((A1<1),(B1<>2))"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[9]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =OR(A1=1,B1=2)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[10]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =OR(TRUE,FALSE)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[11]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =NOT(TRUE)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[12]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =AND(1,DEFINED(ABC1))"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123389)
{
    loadAndSave("tdf123389.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Keep original formula during round-trip
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =ROUND(2.345,1)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =ROUND(A1,2)"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf106953, "tdf106953.docx")
{
    uno::Reference<container::XIndexAccess> xRules = getProperty< uno::Reference<container::XIndexAccess> >(getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), u"NumberingRules"_ustr);
    // This was -635, so the tab of the numbering expanded to a small value instead of matching Word's larger value.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), comphelper::SequenceAsHashMap(xRules->getByIndex(0))[u"FirstLineIndent"_ustr].get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115094v3)
{
    createSwDoc("tdf115094v3.docx");
    {
        SwDoc* pDoc = getSwDoc();
        auto& rSpzFormats = *pDoc->GetSpzFrameFormats();
        auto pFormat = rSpzFormats[0];
        // Without the fix, this has failed with:
        // - Expected: 1991
        // - Actual  : 1883
        // i.e. some unwanted ~-2mm left margin appeared.
        CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(1991), pFormat->GetHoriOrient().GetPos());
    }
    save(mpFilter);
    // floating table is now exported directly without surrounding frame
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblpPr"_ostr, "tblpX"_ostr, u"1996"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblpPr"_ostr, "tblpY"_ostr, u"1064"_ustr);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
