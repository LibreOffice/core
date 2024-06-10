/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>

#include <config_fonts.h>
#include <officecfg/Office/Writer.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/scopeguard.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr) {}

protected:
    void verifyComboBoxExport(bool aComboBoxAsDropDown);
};

DECLARE_OOXMLEXPORT_TEST(testRelorientation, "relorientation.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // This was text::RelOrientation::FRAME, when handling relativeFrom=page, align=right
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xShape, u"HoriOrientRelation"_ustr));

    uno::Reference<drawing::XShapes> xGroup(xShape, uno::UNO_QUERY);
    // This resulted in lang::IndexOutOfBoundsException, as nested groupshapes weren't handled.
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xGroup->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.GroupShape"_ustr, xShapeDescriptor->getShapeType());

    // 'actual child size' = 'group ext' * 'child ext' / 'chExt from group', see section 'chExt' in
    // [MS-OI29500]. Here for width from file 3108960 * 4896 / 4911 = 3099464 EMU. That corresponds to
    // width 8.61cm and 325px in UI in Word and rounds down to 8609 Hmm. Considering scaling of the
    // parent group to the anchor extent (* 3118485 / 3108960) we get a display width of 3108960 EMU
    // = 8636Hmm. FIXME: Expected value is as in LO 7.2. Reason for difference is yet unknown.
    if (isExported())
    {
        uno::Reference<drawing::XShape> xYear(xGroup->getByIndex(1), uno::UNO_QUERY);
        // This was 2, due to incorrect handling of parent transformations inside DML groupshapes.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8662), xYear->getSize().Width);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testBezier)
{
    loadAndReload("bezier.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Check that no shape got lost: a bezier, a line and a text shape.
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapeTextbox, "groupshape-textbox.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // The VML export lost text on textboxes inside groupshapes.
    // The DML export does not, make sure it stays that way.
    CPPUNIT_ASSERT_EQUAL(u"first"_ustr, xShape->getString());
    // This was 16, i.e. inheriting doc default char height didn't work.
    CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(getParagraphOfText(1, xShape->getText()), u"CharHeight"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapePicture, "groupshape-picture.docx")
{
    // Picture in the groupshape got lost, groupshape had only one child.
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.GraphicObjectShape"_ustr, xShapeDescriptor->getShapeType());
}

DECLARE_OOXMLEXPORT_TEST(testAutofit, "autofit.docx")
{
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), u"TextAutoGrowHeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(2), u"TextAutoGrowHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTrackChangesDeletedParagraphMark)
{
    loadAndSave("testTrackChangesDeletedParagraphMark.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:del"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTrackChangesInsertedParagraphMark)
{
    loadAndSave("testTrackChangesInsertedParagraphMark.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:ins"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTrackChangesDeletedTableRow)
{
    loadAndSave("testTrackChangesDeletedTableRow.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:trPr/w:del"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTrackChangesInsertedTableRow)
{
    loadAndSave("testTrackChangesInsertedTableRow.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:trPr/w:ins"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTrackChangesDeletedTableCell)
{
    loadAndSave("testTrackChangesDeletedTableCell.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:tcPr/w:cellDel"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTrackChangesInsertedTableCell)
{
    loadAndSave("testTrackChangesInsertedTableCell.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:tcPr/w:cellIns"_ostr);
}

DECLARE_OOXMLEXPORT_TEST(testTextBoxPictureFill, "textbox_picturefill.docx")
{
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(xFrame, u"FillStyle"_ustr));
    auto xBitmap = getProperty<uno::Reference<awt::XBitmap>>(xFrame, u"FillBitmap"_ustr);
    CPPUNIT_ASSERT(xBitmap.is());
    uno::Reference<graphic::XGraphic> xGraphic(xBitmap, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);
    CPPUNIT_ASSERT(!aGraphic.IsNone());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > 0L);
    CPPUNIT_ASSERT_EQUAL(tools::Long(447), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(528), aGraphic.GetSizePixel().Height());
}

CPPUNIT_TEST_FIXTURE(Test, testFDO73034)
{
    loadAndSave("FDO73034.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:u"_ostr, "val"_ostr).match("single"));
}

CPPUNIT_TEST_FIXTURE(Test, testFDO71834)
{
    loadAndSave("fdo71834.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[2]/w:tc[1]/w:tcPr[1]/w:tcW[1]"_ostr,"type"_ostr, u"dxa"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTrackChangesParagraphProperties)
{
    loadAndSave("testTrackChangesParagraphProperties.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPathChildren(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pPrChange"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testMsoSpt180)
{
    loadAndReload("mso-spt180.docx");
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    const uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(xGroup->getByIndex(0), u"CustomShapeGeometry"_ustr);
    OUString aType;
    for (beans::PropertyValue const & prop : aProps)
        if (prop.Name == "Type")
            aType = prop.Value.get<OUString>();
    // This was exported as borderCallout90, which is an invalid drawingML preset shape string.
    CPPUNIT_ASSERT_EQUAL(u"ooxml-borderCallout1"_ustr, aType);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo73550)
{
    loadAndSave("fdo73550.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    // This was wrap="none".
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:rFonts"_ostr);
}

DECLARE_OOXMLEXPORT_TEST(testPageRelSize, "pagerelsize.docx")
{
    // First shape: width is relative from page, but not height.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xShape, u"RelativeWidthRelation"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, getProperty<sal_Int16>(xShape, u"RelativeHeightRelation"_ustr));

    // Second shape: height is relative from page, but not height.
    xShape = getShape(2);
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xShape, u"RelativeHeightRelation"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, getProperty<sal_Int16>(xShape, u"RelativeWidthRelation"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testRelSizeRound, "rel-size-round.docx")
{
    // This was 9: 9.8 was imported as 9 instead of being rounded to 10.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(10), getProperty<sal_Int16>(getShape(1), u"RelativeHeight"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTestTitlePage, "testTitlePage.docx")
{
    // this has 2 pages in Word
    CPPUNIT_ASSERT_EQUAL(u"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"_ustr, parseDump("/root/page[2]/footer/txt/text()"_ostr));
}

DECLARE_OOXMLEXPORT_TEST(testTableRowDataDisplayedTwice, "table-row-data-displayed-twice.docx")
{
    // fdo#73534: There was a problem for some documents during export.Invalid sectPr getting added
    // because of wrong condition in code.
    // This was the reason for increasing number of pages after RT
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo73556)
{
    loadAndSave("fdo73556.docx");
    /*
    *  The file contains a table with 3 columns
    *  the gridcols are as follows: {1210, 1331, 1210}
    *  whereas the individual cells have {1210, 400, 1210}
    *  The table column separators were taken from the Grid, while
    *  the table width was calculated as 2820 from cells instead
    *  of 3751 from the Grid.
    */
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol"_ostr, 3);
    sal_Int32 tableWidth = 0;
    tableWidth += getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[1]"_ostr, "w"_ostr).toInt32();
    tableWidth += getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[2]"_ostr, "w"_ostr).toInt32();
    tableWidth += getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[3]"_ostr, "w"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3751), tableWidth);
}

CPPUNIT_TEST_FIXTURE(Test, testSegFaultWhileSave)
{
    loadAndSave("test_segfault_while_save.docx");
    // fdo#74499
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6137), getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[2]"_ostr, "w"_ostr).toInt32());
}

CPPUNIT_TEST_FIXTURE(Test, fdo69656)
{
    loadAndSave("Table_cell_auto_width_fdo69656.docx");
    // Changed the UT to check "dxa" instead of "auto"
    // For this particular issue file few cells have width type "auto"
    // LO supports VARIABLE and FIXED width type.
    // If type is VARIABLE LO calculates width as percent of PageSize
    // Else if the width is fixed it uses the width value.
    // After changes for fdo76741 the fixed width is exported as "dxa" for DOCX

    // Check for the width type of table and its cells.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblW"_ostr,"type"_ostr,u"dxa"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo76741)
{
    loadAndSave("fdo76741.docx");

     // There are two issue related to table in the saved(exported) file
     // - the table alignment in saved file is "left" instead of "center"
     // - the table width type in properties is "auto" instead of "dxa"

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "//w:jc"_ostr, "val"_ostr, u"center"_ustr);
    assertXPath(pXmlDoc, "//w:tblW"_ostr, "w"_ostr, u"10081"_ustr);
    assertXPath(pXmlDoc, "//w:tblW"_ostr, "type"_ostr, u"dxa"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo73541)
{
    loadAndSave("fdo73541.docx");
    // fdo#73541: The mirrored margins were not imported and mapped correctly in Page Layout
    // Hence <w:mirrorMargins /> tag was not exported back in settings.xml
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlDoc, "/w:settings/w:mirrorMargins"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo106029)
{
    loadAndSave("fdo106029.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:doNotExpandShiftReturn"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146515)
{
    loadAndSave("tdf146515.odt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:usePrinterMetrics"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO74106)
{
    loadAndSave("FDO74106.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:numFmt"_ostr, "val"_ostr,u"hebrew1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO74215)
{
    loadAndSave("FDO74215.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);
    // tdf#106849 NumPicBullet xShape should not be resized.

    // This is dependent on the running system: see MSWordExportBase::BulletDefinitions
    // FIXME: the size of a bullet is defined by GraphicSize property
    // (stored in SvxNumberFormat::aGraphicSize) so use that for the size
    // (properly convert from 100mm to pt (1 inch is 72 pt, 1 pt is 20 twips).

    // On 96 DPI "width:11.25pt;height:11.25pt"; on 120 DPI "width:9pt;height:9pt"
    const OUString sStyle
        = getXPath(pXmlDoc, "/w:numbering/w:numPicBullet[2]/w:pict/v:shape"_ostr, "style"_ostr);
    {
        OUString sWidth = sStyle.getToken(0, ';');
        CPPUNIT_ASSERT(sWidth.startsWith("width:", &sWidth));
        CPPUNIT_ASSERT(sWidth.endsWith("pt", &sWidth));
        const double fWidth = sWidth.toDouble();
        const double fXScaleFactor = 96.0 / Application::GetDefaultDevice()->GetDPIX();
        // note: used to fail on Mac with 14.7945205479452 vs. 14.8
        // note: used to fail on another Mac with 12.1348314606742 vs 12.15
        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.25 * fXScaleFactor, fWidth, 0.1);
    }
    {
        OUString sHeight = sStyle.getToken(1, ';');
        CPPUNIT_ASSERT(sHeight.startsWith("height:", &sHeight));
        CPPUNIT_ASSERT(sHeight.endsWith("pt", &sHeight));
        const double fHeight = sHeight.toDouble();
        const double fYScaleFactor = 96.0 / Application::GetDefaultDevice()->GetDPIY();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.25 * fYScaleFactor, fHeight, 0.1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testColumnBreak_ColumnCountIsZero,"fdo74153.docx")
{
    /* fdo73545: Column Break with Column_count = 0 was not getting preserved.
     * The <w:br w:type="column" /> was missing after roundtrip
     */
    if (isExported())
    {
        xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:br"_ostr,"type"_ostr,u"column"_ustr);
    }

    //tdf76349 match Word's behavior of treating breaks in single columns as page breaks.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf90697_complexBreaksHeaders,"tdf90697_complexBreaksHeaders.docx")
{
// This is a complex document using many types of section breaks and re-defined headers.
// Paragraphs 44-47 were in two columns
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(45), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

// after that, the section break should switch things back to one column.
    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(50), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());
}

CPPUNIT_TEST_FIXTURE(Test, testIndentation)
{
    loadAndSave("test_indentation.docx");
    // fdo#74141 :There was a problem that in style.xml and document.xml in <w:ind> tag "right" & "left" margin
    // attributes gets added(w:right=0 & w:left=0) if these attributes are not set in original document.
    // This test is to verify <w:ind> does not contain w:right attribute.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:ind"_ostr, "end"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testChartInFooter)
{
    loadAndSave("chart-in-footer.docx");
    // fdo#73872: document contains chart in footer.
    // The problem was that  footer1.xml.rels files for footer1.xml
    // files were missing from docx file after roundtrip.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/_rels/footer2.xml.rels"_ustr);

    // Check footer2.xml.rels contains in doc after roundtrip.
    // Check Id = rId1 in footer2.xml.rels
    assertXPath(pXmlDoc,"/rels:Relationships/rels:Relationship"_ostr,"Id"_ostr,u"rId1"_ustr);
    assertXPath(pXmlDoc,
        "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr,
        "Type"_ostr,
        u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart"_ustr);

    xmlDocUniquePtr pXmlDocCT = parseExport(u"[Content_Types].xml"_ustr);
    assertXPath(pXmlDocCT,
        "/ContentType:Types/ContentType:Override[@PartName='/word/charts/chart1.xml']"_ostr,
        "ContentType"_ostr,
        u"application/vnd.openxmlformats-officedocument.drawingml.chart+xml"_ustr);

    // check the content too
    xmlDocUniquePtr pXmlDocFooter2 = parseExport(u"word/footer2.xml"_ustr);
    assertXPath(pXmlDocFooter2,
        "/w:ftr/w:p[1]/w:r/w:drawing/wp:inline/a:graphic/a:graphicData"_ostr,
        "uri"_ostr,
        u"http://schemas.openxmlformats.org/drawingml/2006/chart"_ustr);
    assertXPath(pXmlDocFooter2,
        "/w:ftr/w:p[1]/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/c:chart"_ostr,
        "id"_ostr,
        u"rId1"_ustr);

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(Test, testNestedTextFrames)
{
    loadAndReload("nested-text-frames.odt");
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // First problem was LO crashed during export (crash test)

    // Second problem was LO made file corruption, writing out nested text boxes, which can't be handled by Word.
    // Test that all three exported text boxes are anchored to the same paragraph and not each other.
    uno::Reference<text::XTextContent> xTextContent(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
    uno::Reference<text::XText> xText = xRange->getText();
    CPPUNIT_ASSERT_EQUAL(u"Anchor point"_ustr, xText->getString());

    xTextContent.set(getShape(2), uno::UNO_QUERY);
    xRange = xTextContent->getAnchor();
    xText = xRange->getText();
    CPPUNIT_ASSERT_EQUAL(u"Anchor point"_ustr, xText->getString());

    xTextContent.set(getShape(3), uno::UNO_QUERY);
    xRange = xTextContent->getAnchor();
    xText = xRange->getText();
    CPPUNIT_ASSERT_EQUAL(u"Anchor point"_ustr, xText->getString());
}

DECLARE_OOXMLEXPORT_TEST(testFloatingTablePosition, "floating-table-position.docx")
{
    // Position of shape was wrong, because some conversion was missing.
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    // This was 3295.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5964), getProperty<sal_Int32>(xShape, u"HoriOrientPosition"_ustr));
    // This was 4611.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8133), getProperty<sal_Int32>(xShape, u"VertOrientPosition"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testAbi11739)
{
    loadAndSave("abi11739.docx");
    // Validation test: order of elements were wrong.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);
    // Order was: uiPriority, link, basedOn.
    CPPUNIT_ASSERT(getXPathPosition(pXmlDoc, "/w:styles/w:style[3]"_ostr, "basedOn") < getXPathPosition(pXmlDoc, "/w:styles/w:style[3]"_ostr, "link"));
    CPPUNIT_ASSERT(getXPathPosition(pXmlDoc, "/w:styles/w:style[3]"_ostr, "link") < getXPathPosition(pXmlDoc, "/w:styles/w:style[3]"_ostr, "uiPriority"));
    // Order was: qFormat, unhideWhenUsed.
    CPPUNIT_ASSERT(getXPathPosition(pXmlDoc, "/w:styles/w:style[11]"_ostr, "unhideWhenUsed") < getXPathPosition(pXmlDoc, "/w:styles/w:style[11]"_ostr, "qFormat"));
}

DECLARE_OOXMLEXPORT_TEST(testEmbeddedXlsx, "embedded-xlsx.docx")
{
    // check there are two objects and they are FrameShapes
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(u"FrameShape"_ustr, getShape(1)->getShapeType());
    CPPUNIT_ASSERT_EQUAL(u"FrameShape"_ustr, getShape(2)->getShapeType());

    // check the objects are present in the exported document.xml
    if (!isExported())
        return;
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/w:object"_ostr, 2);

    // finally check the embedded files are present in the zipped document
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
    const uno::Sequence<OUString> names = xNameAccess->getElementNames();
    int nSheetFiles = 0;
    int nImageFiles = 0;
    for (OUString const & n : names)
    {
        if(n.startsWith("word/embeddings/oleObject"))
            nSheetFiles++;
        if(n.startsWith("word/media/image"))
            nImageFiles++;
    }
    CPPUNIT_ASSERT_EQUAL(2, nSheetFiles);
    CPPUNIT_ASSERT_EQUAL(2, nImageFiles);
}

CPPUNIT_TEST_FIXTURE(Test, testNumberedLists_StartingWithZero)
{
    loadAndSave("FDO74105.docx");
    /* Issue : Numbered lists Starting with value '0' is not preserved after RT.
     * In numbering.xml, an XML tag <w:start> is optional. If not mentioned,
     * the Numbered list should start from 0.
     * Problem was LO was writing <w:start> for all levels 0-8 with default value "1".
     */
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);

    // Check that we do _not_ export w:start for <w:lvl w:ilvl="0">.
    assertXPath(pXmlDoc, "w:numbering/w:abstractNum[1]/w:lvl[1]/w:start"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testPageBreak)
{
    loadAndReload("fdo74566.docx");
    /*  Break to next page was written into wrong paragraph as <w:pageBreakBefore />.
     *  LO was not preserving Page Break as <w:br w:type="page" />.
     *  Now after fix , LO writes Page Break as the new paragraph and also
     *  preserves the xml tag <w:br>.
     */
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    uno::Reference<text::XTextRange> xParagraph2 = getParagraph(2);
    uno::Reference<text::XTextRange> xParagraph4 = getParagraph(4);

    getRun(xParagraph2, 1, u"First Page Second Line"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:br"_ostr,"type"_ostr,u"page"_ustr);
    getRun(xParagraph4, 1, u"Second Page First line after Page Break"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testOleObject)
{
    loadAndSave("test_ole_object.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:object/v:shape/v:imagedata"_ostr,
                           "o:title"_ostr);
    assertXPath(pXmlDoc,
        "/w:document/w:body/w:p[2]/w:r/w:object/o:OLEObject"_ostr,
        "DrawAspect"_ostr,
        u"Content"_ustr);
    // TODO: ProgID="Package" - what is this? Zip with 10k extra header?

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.bin']"_ostr,
        "Type"_ostr,
        u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject"_ustr);
    // check the media type too
    xmlDocUniquePtr pXmlDocCT = parseExport(u"[Content_Types].xml"_ustr);
    assertXPath(pXmlDocCT,
        "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.bin']"_ostr,
        "ContentType"_ostr,
        u"application/vnd.openxmlformats-officedocument.oleObject"_ustr);

}

CPPUNIT_TEST_FIXTURE(Test, testFdo74792)
{
    loadAndSave("fdo74792.docx");
    /*
     * fdo#74792 : The images associated with smart-art data[i].xml
     * were not preserved on exporting to DOCX format
     * Added support to grabbag the rels, with associated images.
     */
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/diagrams/_rels/data1.xml.rels"_ustr);
    assertXPath(pXmlDoc,"/rels:Relationships/rels:Relationship"_ostr, 4);
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(
                         comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());

    //check that images are also saved
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName( u"word/media/OOXDiagramDataRels1_0.jpeg"_ustr /*added anchor id to form a unique name*/ ), uno::UNO_QUERY);
    CPPUNIT_ASSERT( xInputStream.is() );
}

CPPUNIT_TEST_FIXTURE(Test, testFdo77718)
{
    loadAndSave("fdo77718.docx");
    //in case of multiple smart arts the names for images were getting
    //repeated and thereby causing a data loss as the binary stream was
    //getting over written. This test case ensures that unique names are
    //given for images in different smart arts.
    xmlDocUniquePtr pXmlDataRels1 = parseExport(u"word/diagrams/_rels/data1.xml.rels"_ustr);
    xmlDocUniquePtr pXmlDataRels2 = parseExport(u"word/diagrams/_rels/data2.xml.rels"_ustr);

    //ensure that the rels file is present.
    assertXPath(pXmlDataRels1,"/rels:Relationships/rels:Relationship"_ostr, 4);
    assertXPath(pXmlDataRels2,"/rels:Relationships/rels:Relationship"_ostr, 4);

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(
                         comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());

    //check that images are also saved
    uno::Reference<io::XInputStream> xInputStream1(xNameAccess->getByName( u"word/media/OOXDiagramDataRels1_0.jpeg"_ustr /*added anchor id to form a unique name*/ ), uno::UNO_QUERY);
    CPPUNIT_ASSERT( xInputStream1.is() );

    //check that images are saved for other smart-arts as well.
    uno::Reference<io::XInputStream> xInputStream2(xNameAccess->getByName( u"word/media/OOXDiagramDataRels2_0.jpeg"_ustr /*added anchor id to form a unique name*/ ), uno::UNO_QUERY);
    CPPUNIT_ASSERT( xInputStream2.is() );
}

CPPUNIT_TEST_FIXTURE(Test, testTableCurruption)
{
    loadAndSave("tableCurrupt.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/header2.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc) ;
    assertXPath(pXmlDoc, "/w:hdr/w:tbl[1]/w:tr[1]/w:tc[1]"_ostr,1);

    // tdf#116549: header paragraph should not have a bottom border.
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xStyle.is());
    uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(xStyle, u"HeaderTextFirst"_ustr);
    CPPUNIT_ASSERT(xHeaderText.is());
    auto xParagraph = getParagraphOfText(1, xHeaderText);
    CPPUNIT_ASSERT(xParagraph.is());
    table::BorderLine2 aHeaderBottomBorder = getProperty<table::BorderLine2>(xParagraph, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), aHeaderBottomBorder.LineWidth);
}

CPPUNIT_TEST_FIXTURE(Test, testDateControl)
{
    loadAndSave("date-control.docx");
    // check XML
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:date"_ostr, "fullDate"_ostr, u"2014-03-05T00:00:00Z"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:date/w:dateFormat"_ostr, "val"_ostr, u"dddd, dd' de 'MMMM' de 'yyyy"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:date/w:lid"_ostr, "val"_ostr, u"es-ES"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r/w:t"_ostr, u"mi\u00E9rcoles, 05 de marzo de 2014"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, test_Tdf115030)
{
    loadAndSave("tdf115030.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    sal_Unicode aDot = {0x02D9};
    sal_Unicode aDobleDot = {0x00A8};
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/m:oMath[1]/m:acc/m:accPr/m:chr"_ostr, "val"_ostr, OUString(aDot));
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/m:oMath[2]/m:acc/m:accPr/m:chr"_ostr, "val"_ostr, OUString(aDobleDot));
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/m:oMathPara/m:oMath[1]/m:acc/m:accPr/m:chr"_ostr, "val"_ostr, OUString(aDot));
}

CPPUNIT_TEST_FIXTURE(Test, test_OpeningBrace)
{
    loadAndSave("2120112713_OpenBrace.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Checking for OpeningBrace tag
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/m:oMath[1]/m:d[1]/m:dPr[1]/m:begChr[1]"_ostr,"val"_ostr,u""_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, test_Tdf132305)
{
    loadAndSave("tdf132305.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/m:oMath[1]/m:bar/m:barPr/m:pos"_ostr,"val"_ostr,u"top"_ustr);
}

// Checks that all runs of the field have text properties.
// Old behaviour: only first run has text properties of the field
//
// There are several runs are used in fields:
//     <w:r>
//         <w:rPr>
//             <!-- properties written with DocxAttributeOutput::StartRunProperties() / DocxAttributeOutput::EndRunProperties().
//         </w:rPr>
//         <w:fldChar w:fldCharType="begin" />
//     </w:r>
//         <w:r>
//         <w:rPr>
//             <!-- properties written with DocxAttributeOutput::DoWriteFieldRunProperties()
//         </w:rPr>
//         <w:instrText>TIME \@"HH:mm:ss"</w:instrText>
//     </w:r>
//     <w:r>
//         <w:rPr>
//             <!-- properties written with DocxAttributeOutput::DoWriteFieldRunProperties()
//         </w:rPr>
//         <w:fldChar w:fldCharType="separate" />
//     </w:r>
//     <w:r>
//         <w:rPr>
//             <!-- properties written with DocxAttributeOutput::DoWriteFieldRunProperties()
//         </w:rPr>
//         <w:t>14:01:13</w:t>
//         </w:r>
//     <w:r>
//         <w:rPr>
//             <!-- properties written with DocxAttributeOutput::DoWriteFieldRunProperties()
//         </w:rPr>
//         <w:fldChar w:fldCharType="end" />
//     </w:r>
// See, tdf#38778
CPPUNIT_TEST_FIXTURE(Test, testTdf38778)
{
    loadAndSave("tdf38778_properties_in_run_for_field.doc");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    static constexpr OUString psz(u"20"_ustr);
    static constexpr OUString pszCs(u"20"_ustr);

    // w:fldCharType="begin"
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w:sz"_ostr,   "val"_ostr, psz);
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w:szCs"_ostr, "val"_ostr, pszCs);

    // PAGE
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[4]/w:rPr/w:sz"_ostr,   "val"_ostr, psz);
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[4]/w:rPr/w:szCs"_ostr, "val"_ostr, pszCs);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[4]/w:instrText"_ostr,  u" PAGE "_ustr);

    // w:fldCharType="separate"
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[5]/w:rPr/w:sz"_ostr,   "val"_ostr, psz);
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[5]/w:rPr/w:szCs"_ostr, "val"_ostr, pszCs);

    // field result: 1
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[6]/w:rPr/w:sz"_ostr,   "val"_ostr, psz);
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[6]/w:rPr/w:szCs"_ostr, "val"_ostr, pszCs);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[6]/w:t"_ostr,          u"1"_ustr); // field result

    // w:fldCharType="end"
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[7]/w:rPr/w:sz"_ostr,   "val"_ostr, psz);
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[7]/w:rPr/w:szCs"_ostr, "val"_ostr, pszCs);

    // tdf#127862: page fill color (in this case white) was lost
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT(drawing::FillStyle_NONE != getProperty<drawing::FillStyle>(xStyle, u"FillStyle"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testFDO76312)
{
    loadAndSave("FDO76312.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]"_ostr);
}

void Test::verifyComboBoxExport(bool aComboBoxAsDropDown)
{
    if (aComboBoxAsDropDown)
    {
        // ComboBox was imported as DropDown text field
        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
        CPPUNIT_ASSERT(xFields->hasMoreElements());
        uno::Any aField = xFields->nextElement();
        uno::Reference<lang::XServiceInfo> xServiceInfo(aField, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.textfield.DropDown"_ustr));

        uno::Sequence<OUString> aItems = getProperty< uno::Sequence<OUString> >(aField, u"Items"_ustr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aItems.getLength());
        CPPUNIT_ASSERT_EQUAL(u"manolo"_ustr, aItems[0]);
        CPPUNIT_ASSERT_EQUAL(u"pepito"_ustr, aItems[1]);
        CPPUNIT_ASSERT_EQUAL(u"Manolo"_ustr, aItems[2]);
    }
    else
    {
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<drawing::XControlShape> xControl(xShape, uno::UNO_QUERY);

        CPPUNIT_ASSERT_EQUAL(u"Manolo"_ustr, getProperty<OUString>(xControl->getControl(), u"Text"_ustr));

        uno::Sequence<OUString> aItems = getProperty< uno::Sequence<OUString> >(xControl->getControl(), u"StringItemList"_ustr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aItems.getLength());
        CPPUNIT_ASSERT_EQUAL(u"manolo"_ustr, aItems[0]);
        CPPUNIT_ASSERT_EQUAL(u"pepito"_ustr, aItems[1]);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testComboBoxControl)
{
    loadAndSave("combobox-control.docx");
    // check XML
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dropDownList/w:listItem[1]"_ostr, "value"_ostr, u"manolo"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dropDownList/w:listItem[2]"_ostr, "value"_ostr, u"pepito"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r/w:t"_ostr, u"Manolo"_ustr);

    // check imported control
    verifyComboBoxExport(getShapes() == 0);
}

CPPUNIT_TEST_FIXTURE(Test, tdf134043_ImportComboBoxAsDropDown_true)
{
    createSwDoc("combobox-control.docx");
    verifyComboBoxExport(true);
}

CPPUNIT_TEST_FIXTURE(Test, tdf134043_ImportComboBoxAsDropDown_false)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::Filter::Import::DOCX::ImportComboBoxAsDropDown::set(false, batch);
    batch->commit();
    comphelper::ScopeGuard g(
        [batch]
        {
            officecfg::Office::Writer::Filter::Import::DOCX::ImportComboBoxAsDropDown::set(true,
                                                                                           batch);
            batch->commit();
        });

    createSwDoc("combobox-control.docx");
    verifyComboBoxExport(false);
}

CPPUNIT_TEST_FIXTURE(Test, testCheckBoxControl)
{
    loadAndSave("checkbox-control.docx");
    // check XML
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w14:checkbox/w14:checked"_ostr, "val"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w14:checkbox/w14:checkedState"_ostr, "val"_ostr, u"2612"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w14:checkbox/w14:uncheckedState"_ostr, "val"_ostr, u"2610"_ustr);

    // TODO: import control and add a check here
}

CPPUNIT_TEST_FIXTURE(Test, testParagraphWithComments)
{
    loadAndSave("paragraphWithComments.docx");
    /* Comment id's were getting overwritten for annotation mark(s),
       which was causing a mismatch in the relationship for comment id's
       in document.xml and comment.xml
    */
    xmlDocUniquePtr pXmlDoc  = parseExport(u"word/document.xml"_ustr);
    xmlDocUniquePtr pXmlComm = parseExport(u"word/comments.xml"_ustr);

    sal_Int32 idInDocXml     = getXPath(pXmlDoc,"/w:document/w:body/w:p[3]/w:commentRangeEnd[1]"_ostr,"id"_ostr).toInt32();
    sal_Int32 idInCommentXml = getXPath(pXmlComm,"/w:comments/w:comment[1]"_ostr,"id"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL( idInDocXml, idInCommentXml );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104707_urlComment)
{
    loadAndReload("tdf104707_urlComment.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    auto aField1 = xFields->nextElement();
    // the comment/annotation/postit text
    auto xText = getProperty< uno::Reference<text::XText> >(aField1, u"TextRange"_ustr);
    // the hyperlink within the comment text
    auto xURLField = getProperty< uno::Reference<text::XTextField> >(xText, u"TextField"_ustr);
    auto aURL = getProperty< OUString >(xURLField, u"URL"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"https://bugs.documentfoundation.org/show_bug.cgi?id=104707"_ustr, aURL);
}

CPPUNIT_TEST_FIXTURE(Test, testOLEObjectinHeader)
{
    loadAndSave("2129393649.docx");
    // fdo#76015 : Document contains oleobject in header xml.
    // Problem was relationship entry for oleobject from header was
    // exported into document.xml.rels file because of this rels file
    // for headers were missing from document/word/rels.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/_rels/header2.xml.rels"_ustr);

    assertXPath(pXmlDoc,"/rels:Relationships/rels:Relationship[1]"_ostr,"Id"_ostr,u"rId1"_ustr);

    xmlDocUniquePtr pXmlDocCT = parseExport(u"[Content_Types].xml"_ustr);

    // check the media type too
    assertXPath(pXmlDocCT,
        "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.bin']"_ostr,
        "ContentType"_ostr,
        u"application/vnd.openxmlformats-officedocument.oleObject"_ustr);

    // check the content too
    xmlDocUniquePtr pXmlDocHeader2 = parseExport(u"word/header2.xml"_ustr);
    assertXPath(pXmlDocHeader2,
        "/w:hdr/w:tbl/w:tr[1]/w:tc[2]/w:p[1]/w:r/w:object/o:OLEObject"_ostr,
        "ProgID"_ostr,
        u"Word.Picture.8"_ustr);
    xmlDocUniquePtr pXmlDocHeader3 = parseExport(u"word/header3.xml"_ustr);
    assertXPath(pXmlDocHeader2,
        "/w:hdr/w:tbl/w:tr[1]/w:tc[2]/w:p[1]/w:r/w:object/o:OLEObject"_ostr,
        "ProgID"_ostr,
        u"Word.Picture.8"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, test_ClosingBrace)
{
    loadAndSave("2120112713.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Checking for ClosingBrace tag
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/m:oMath[1]/m:d[2]/m:dPr[1]/m:endChr[1]"_ostr,"val"_ostr,u""_ustr);

    // tdf#154371 paragraph style Подзаголовок ур3 inherits from Heading 3 (includes list level and list id)
    uno::Reference<beans::XPropertySet> xParaStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Подзаголовок ур3"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xParaStyle, u"NumberingLevel"_ustr));
    CPPUNIT_ASSERT(!getProperty<OUString>(xParaStyle, u"NumberingStyleName"_ustr).isEmpty());
}

CPPUNIT_TEST_FIXTURE(Test, testlvlPicBulletId)
{
    loadAndSave("lvlPicBulletId.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);
    assertXPath(pXmlDoc, "/w:numbering[1]/w:abstractNum[1]/w:lvl[1]/w:lvlPicBulletId[1]"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtContent)
{
    loadAndSave("SdtContent.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/header1.xml"_ustr);
//    assertXPath(pXmlDoc, "/w:hdr[1]/w:p[1]/w:sdt/w:sdtContent[1]/w:del[1]");
}

#if 0
// Currently LibreOffice exports custom geometry for this up arrow, not preset shape.
// When LibreOffice can export preset shape with correct modifiers, then this test can be re-enabled.

CPPUNIT_TEST_FIXTURE(Test, testFdo76016)
{
    loadAndSave("fdo76016.docx");
    // check XML
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "//a:graphic/a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]", "name", "adj1");
    assertXPath(pXmlDoc, "//a:graphic/a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]", "name", "adj2");
}
#endif

CPPUNIT_TEST_FIXTURE(Test, testFileWithInvalidImageLink)
{
    loadAndSave("FileWithInvalidImageLink.docx");
    /* In case if the original file has an image whose link is
       invalid, then the RT file used to result in corruption
       since the exported image would be an empty image.
     */
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPathNoAttribute(pXmlDoc,
                           "/w:document/w:body/w:p[2]/w:r[2]/w:drawing[1]/wp:inline[1]/"
                           "a:graphic[1]/a:graphicData[1]/pic:pic[1]/pic:blipFill[1]/a:blip[1]"_ostr,
                           "embed"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testContentTypeDOCX)
{
    loadAndSave("fdo80410.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);

    assertXPath(pXmlDoc,
        "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.docx']"_ostr,
        "ContentType"_ostr,
        u"application/vnd.openxmlformats-officedocument.wordprocessingml.document"_ustr);
    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.docx']"_ostr,
        "Type"_ostr,
        u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package"_ustr);
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[6]/w:r/w:object/o:OLEObject"_ostr,
        "ProgID"_ostr,
        u"Word.Document.12"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testContentTypeXLSM)
{
    loadAndSave("fdo76098.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);

    assertXPath(pXmlDoc, "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/Microsoft_Excel_Macro-Enabled_Worksheet1.xlsm']"_ostr, "ContentType"_ostr, u"application/vnd.ms-excel.sheet.macroEnabled.12"_ustr);

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/charts/_rels/chart1.xml.rels"_ustr);
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='../embeddings/Microsoft_Excel_Macro-Enabled_Worksheet1.xlsm']"_ostr,
        "Type"_ostr,
        u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package"_ustr);
    // check the content too
    xmlDocUniquePtr pXmlDocChart1 = parseExport(u"word/charts/chart1.xml"_ustr);
    assertXPath(pXmlDocChart1,
        "/c:chartSpace/c:externalData"_ostr,
        "id"_ostr,
        u"rId1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, test76108)
{
    loadAndSave("test76108.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    //docx file after RT is getting corrupted.
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/w:fldChar[1]"_ostr, "fldCharType"_ostr, u"begin"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTCTagMisMatch)
{
    loadAndSave("TCTagMisMatch.docx");
    // TCTagMisMatch.docx : This document contains an empty table with borders.
    // there was a TC tag mismatch which resulted into a crash.

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc,"/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl[1]/w:tr[1]/w:tc[1]"_ostr,0);
    assertXPath(pXmlDoc,"/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO78292)
{
    loadAndSave("FDO78292.docx");
    //text node is a leaf node, it should not have any children
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[14]/w:sdt[3]/w:sdtPr[1]/w:text/w14:checked"_ostr,0);
}

CPPUNIT_TEST_FIXTURE(Test, testSimpleSdts)
{
    loadAndSave("simple-sdts.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:sdt/w:sdtPr/w:text"_ostr, 1);
    assertXPath(pXmlDoc, "//*/w:sdt/w:sdtPr/w:id"_ostr, 5);
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:lock"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt[1]/w:sdtPr/w:picture"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt[2]/w:sdtPr/w:group"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:sdt/w:sdtPr/w:citation"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testEmbeddedExcelChart)
{
    loadAndSave("EmbeddedExcelChart.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);

    assertXPath(pXmlDoc,
        "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.xls']"_ostr,
        "ContentType"_ostr,
        u"application/vnd.ms-excel"_ustr);

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xls']"_ostr,
        "Type"_ostr,
        u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject"_ustr);

    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p/w:r/w:object/o:OLEObject"_ostr,
        "ProgID"_ostr,
        u"Excel.Chart.8"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf83227)
{
    loadAndReload("tdf83227.docx");
    // Bug document contains a rotated image, which is handled as a draw shape (not as a Writer image) on export.
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName(u"word/media/image1.png"_ustr)));
    // This was also true, image was written twice.
    CPPUNIT_ASSERT_EQUAL(false, bool(xNameAccess->hasByName(u"word/media/image2.png"_ustr)));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf103001)
{
    loadAndReload("tdf103001.docx");
    // The same image is featured in the header and in the body text, make sure
    // the header relation is still written, even when caching is enabled.
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
    // This failed: header reused the RelId of the body text, even if RelIds
    // are local to their stream.
    CPPUNIT_ASSERT(xNameAccess->hasByName(u"word/_rels/header2.xml.rels"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf92521)
{
    loadAndSave("tdf92521.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // There should be a section break that's in the middle of the document: right after the table.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:sectPr"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf102466, "tdf102466.docx")
{
    // the problem was: file is truncated: the first page is missing.
    // More precisely, the table in the first page was clipped.
    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        sal_Int32 nFlyPrtHeight = getXPath(pXmlDoc, "(/root/page[1]//anchored/fly)[1]/infos/prtBounds"_ostr, "height"_ostr).toInt32();
        sal_Int32 nTableHeight = getXPath(pXmlDoc, "(/root/page[1]//anchored/fly)[1]/tab/infos/bounds"_ostr, "height"_ostr).toInt32();
        CPPUNIT_ASSERT_MESSAGE("The table is clipped in a fly frame.", nFlyPrtHeight >= nTableHeight);
    }

    // check how much pages we have: it should match the Word layout result
    CPPUNIT_ASSERT_EQUAL(11, getPages());

    // check content of the first page
    {
        uno::Reference<beans::XPropertySet> xFrame(getShapeByName(u"Marco1"), uno::UNO_QUERY);

        // no border
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xFrame, u"LineWidth"_ustr));
    }

    // Make sure we have 19 tables created
    {
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xTables->getCount() >= sal_Int32(19)); // TODO

        // check the text inside first cell of the first table
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);

        const OUString aActualText = xCell->getString();

        CPPUNIT_ASSERT(aActualText.indexOf("Requerimientos del  Cliente") > 0);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99090_pgbrkAfterTable)
{
    loadAndSave("tdf99090_pgbrkAfterTable.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // There should be a regular page break that's in the middle of the document: right after the table.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:br"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf96750_landscapeFollow, "tdf96750_landscapeFollow.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStyle, u"IsLandscape"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf86926_A3, "tdf86926_A3.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(42000), getProperty<sal_Int32>(xStyle, u"Height"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(29700), getProperty<sal_Int32>(xStyle, u"Width"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf64372_continuousBreaks,"tdf64372_continuousBreaks.docx")
{
    //There are no page breaks, so everything should be on the first page.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf92724_continuousBreaksComplex,"tdf92724_continuousBreaksComplex.docx")
{
    //There are 2 page breaks, so there should be 3 pages.
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf90697_continuousBreaksComplex2,"tdf92724_continuousBreaksComplex2.docx")
{
// Continuous section breaks with new headers/footers should not immediately switch to a new page style.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();

    sal_Int16 nPages = xCursor->getPage();
    while( nPages > 0 )
    {
        OUString sPageStyleName = getProperty<OUString>( xCursor, u"PageStyleName"_ustr );
        uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles(u"PageStyles"_ustr)->getByName(sPageStyleName), u"HeaderText"_ustr);
// Specific case to avoid.  Testing separately (even though redundant).
// The first header (defined on page 3) ONLY is shown IF the section happens to start on a new page (which it doesn't in this document).
        CPPUNIT_ASSERT( xHeaderText->getString() != "Third section - First page header. No follow defined" );
// Same test stated differently: Pages 4 and 5 OUGHT to use "Second section header", but currently don't.  Page 6 does.
        if( nPages <= 3 )
            CPPUNIT_ASSERT_EQUAL( u"First section header"_ustr, xHeaderText->getString() );
        else
            CPPUNIT_ASSERT( xHeaderText->getString() == "First section header" || xHeaderText->getString() == "Second section header" );

        xCursor->jumpToPage( --nPages );
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf95367_inheritFollowStyle, "tdf95367_inheritFollowStyle.docx")
{
    CPPUNIT_ASSERT_EQUAL(u"header"_ustr,  parseDump("/root/page[2]/header/txt/text()"_ostr));
}

DECLARE_OOXMLEXPORT_TEST(testInheritFirstHeader,"inheritFirstHeader.docx")
{
// First page headers always link to last used first header, never to a follow header
    CPPUNIT_ASSERT_EQUAL(u"First Header"_ustr, parseDump("/root/page[1]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"Follow Header"_ustr, parseDump("/root/page[2]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"Follow Header"_ustr, parseDump("/root/page[3]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"First Header"_ustr, parseDump("/root/page[4]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"Last Header"_ustr, parseDump("/root/page[5]/header/txt/text()"_ostr));
}

#if HAVE_MORE_FONTS
DECLARE_OOXMLEXPORT_TEST(testTdf81345_045Original, "tdf81345.docx")
{
    //Header wasn't replaced  and columns were missing because no new style was created.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);

    xCursor->jumpToLastPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, u"PageStyleName"_ustr);
    CPPUNIT_ASSERT(pageStyleName != "Standard");

    // tdf89297 Styles were being added before their base/parent/inherited-from style existed, and so were using default settings.
    uno::Reference<container::XNameAccess> xParaStyles(getStyles(u"ParagraphStyles"_ustr));
    uno::Reference<beans::XPropertySet> xStyle(xParaStyles->getByName(u"Pull quote"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6736947), getProperty<sal_Int32>(xStyle, u"CharColor"_ustr));
}
#endif

CPPUNIT_TEST_FIXTURE(Test, testDocxTablePosition)
{
    loadAndSave("floating-table-position.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // the exported positions were wrong due to some missing shifting in the export code
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblpPr"_ostr, "tblpX"_ostr, u"3494"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblpPr"_ostr, "tblpY"_ostr, u"4611"_ustr);
}
#if 0
// FIXME:
CPPUNIT_TEST_FIXTURE(Test, testUnderlineGroupShapeText)
{
    loadAndSave("tdf123351_UnderlineGroupSapeText.docx");
    // tdf#123351: Check if correct underline is used.
    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "single");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "double");

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "thick");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "dotted");

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "dottedHeavy");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "dash");

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[4]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "dashedHeavy");

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[5]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r[1]/w:rPr/w:u", "val", "dashLongHeavy");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[5]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "dotDash");

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[6]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "dashDotHeavy");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[6]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "dotDotDash");

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[7]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "dashDotDotHeavy");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[7]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "wave");

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[8]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "wavyHeavy");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent[8]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "wavyDouble");

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[32]/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "none");

    // TODO: Import of "words".
    // This must fail when import of "words" is implemented. This is a temporary solution, we read "words" as "single".
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[32]/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u", "val", "single");
}
#endif
CPPUNIT_TEST_FIXTURE(Test, testUnderlineColorGroupedShapes)
{
    loadAndSave("tdf132491_UnderlineColorGroupedShapes.docx");
    // tdf#132491 : Check if correct color is used for underline.
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u"_ostr, "color"_ostr, u"FF0000"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u"_ostr, "color"_ostr, u"00B050"_ustr);
    assertXPathNoAttribute(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[3]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:u"_ostr, "color"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testRelativeAnchorWidthFromRightMargin)
{
    loadAndSave("tdf133670_testRelativeAnchorWidthFromRightMargin.docx");
    // tdf#133670 The width was set relative from right margin, but this was handled relative from page width.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    auto nWidth = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "width"_ostr).toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2408, nWidth, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testAutoFitForLegacyShapes)
{
    loadAndSave("tdf112312_AutoFitForLegacyShapes.odt");
    // tdf#112312: check if noAutoFit is used instead of spAutoFit even if the TextAutoGrowHeight is set
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:bodyPr/a:noAutofit"_ostr);
    assertXPathNoAttribute(pXmlDocument, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:bodyPr"_ostr, "a:spAutofit"_ostr);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
