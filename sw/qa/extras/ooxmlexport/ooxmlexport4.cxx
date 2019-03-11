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
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <unotools/tempfile.hxx>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <config_features.h>
#include <string>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        const char* aBlacklist[] = {
            "math-escape.docx",
            "math-mso2k7.docx",
        };
        std::vector<const char*> vBlacklist(aBlacklist, aBlacklist + SAL_N_ELEMENTS(aBlacklist));

        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".docx") && std::find(vBlacklist.begin(), vBlacklist.end(), filename) == vBlacklist.end());
    }
};

DECLARE_OOXMLEXPORT_TEST(testRelorientation, "relorientation.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // This was text::RelOrientation::FRAME, when handling relativeFrom=page, align=right
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xShape, "HoriOrientRelation"));

    uno::Reference<drawing::XShapes> xGroup(xShape, uno::UNO_QUERY);
    // This resulted in lang::IndexOutOfBoundsException, as nested groupshapes weren't handled.
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xGroup->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GroupShape"), xShapeDescriptor->getShapeType());

    // Right after import we get a rounding error: 8662 vs 8664.
    if (mbExported)
    {
        uno::Reference<drawing::XShape> xYear(xGroup->getByIndex(1), uno::UNO_QUERY);
        // This was 2, due to incorrect handling of parent transformations inside DML groupshapes.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8664), xYear->getSize().Width);
    }
}

DECLARE_OOXMLEXPORT_TEST(testBezier, "bezier.odt")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // Check that no shape got lost: a bezier, a line and a text shape.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xDraws->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapeTextbox, "groupshape-textbox.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // The VML export lost text on textboxes inside groupshapes.
    // The DML export does not, make sure it stays that way.
    CPPUNIT_ASSERT_EQUAL(OUString("first"), xShape->getString());
    // This was 16, i.e. inheriting doc default char height didn't work.
    CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xShape, "CharHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapePicture, "groupshape-picture.docx")
{
    // Picture in the groupshape got lost, groupshape had only one child.
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GraphicObjectShape"), xShapeDescriptor->getShapeType());
}

DECLARE_OOXMLEXPORT_TEST(testAutofit, "autofit.docx")
{
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), "TextAutoGrowHeight"));
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(2), "TextAutoGrowHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesDeletedParagraphMark, "testTrackChangesDeletedParagraphMark.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:del");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesInsertedParagraphMark, "testTrackChangesInsertedParagraphMark.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:ins");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesDeletedTableRow, "testTrackChangesDeletedTableRow.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:trPr/w:del");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesInsertedTableRow, "testTrackChangesInsertedTableRow.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:trPr/w:ins");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesDeletedTableCell, "testTrackChangesDeletedTableCell.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:tcPr/w:cellDel");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesInsertedTableCell, "testTrackChangesInsertedTableCell.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:tcPr/w:cellIns");
}

DECLARE_OOXMLEXPORT_TEST(testTextBoxPictureFill, "textbox_picturefill.docx")
{
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    auto xBitmap = getProperty<uno::Reference<awt::XBitmap>>(xFrame, "FillBitmap");
    CPPUNIT_ASSERT(xBitmap.is());
    uno::Reference<graphic::XGraphic> xGraphic(xBitmap, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);
    CPPUNIT_ASSERT(aGraphic);
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > 0L);
    CPPUNIT_ASSERT_EQUAL(447L, aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(528L, aGraphic.GetSizePixel().Height());
}

DECLARE_OOXMLEXPORT_TEST(testFDO73034, "FDO73034.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:u", "val").match("single"));
}

DECLARE_OOXMLEXPORT_TEST(testFDO71834, "fdo71834.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[2]/w:tc[1]/w:tcPr[1]/w:tcW[1]","type", "dxa");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesParagraphProperties, "testTrackChangesParagraphProperties.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPathChildren(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pPrChange", 0);
}

DECLARE_OOXMLEXPORT_TEST(testMsoSpt180, "mso-spt180.docx")
{
    if (!mbExported)
        return;

    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(xGroup->getByIndex(0), "CustomShapeGeometry");
    OUString aType;
    for (int i = 0; i < aProps.getLength(); ++i)
        if (aProps[i].Name == "Type")
            aType = aProps[i].Value.get<OUString>();
    // This was exported as borderCallout90, which is an invalid drawingML preset shape string.
    CPPUNIT_ASSERT_EQUAL(OUString("ooxml-borderCallout1"), aType);
}

DECLARE_OOXMLEXPORT_TEST(testFdo73550, "fdo73550.docx")
{
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;
    // This was wrap="none".
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:rFonts");
}

DECLARE_OOXMLEXPORT_TEST(testPageRelSize, "pagerelsize.docx")
{
    // First shape: width is relative from page, but not height.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xShape, "RelativeWidthRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, getProperty<sal_Int16>(xShape, "RelativeHeightRelation"));

    // Second shape: height is relative from page, but not height.
    xShape = getShape(2);
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xShape, "RelativeHeightRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, getProperty<sal_Int16>(xShape, "RelativeWidthRelation"));
}

DECLARE_OOXMLEXPORT_TEST(testRelSizeRound, "rel-size-round.docx")
{
    // This was 9: 9.8 was imported as 9 instead of being rounded to 10.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(10), getProperty<sal_Int16>(getShape(1), "RelativeHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testTestTitlePage, "testTitlePage.docx")
{
    CPPUNIT_ASSERT_EQUAL(OUString("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"), parseDump("/root/page[last()]/footer/txt/text()"));
}

DECLARE_OOXMLEXPORT_TEST(testTableRowDataDisplayedTwice,"table-row-data-displayed-twice.docx")
{
    // fdo#73534: There was a problem for some documents during export.Invalid sectPr getting added
    // because of wrong condition in code.
    // This was the reason for increasing number of pages after RT
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testFdo73556,"fdo73556.docx")
{
    /*
    *  The file contains a table with 3 columns
    *  the gridcols are as follows: {1210, 1331, 1210}
    *  whereas the individual cells have {1210, 400, 1210}
    *  The table column separators were taken from the Grid, while
    *  the table width was calculated as 2820 from cells instead
    *  of 3751 from the Grid.
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol", 3);
    sal_Int32 tableWidth = 0;
    tableWidth += getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[1]", "w").toInt32();
    tableWidth += getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[2]", "w").toInt32();
    tableWidth += getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[3]", "w").toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3751), tableWidth);
}

DECLARE_OOXMLEXPORT_TEST(testSegFaultWhileSave, "test_segfault_while_save.docx")
{
    // fdo#74499
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6138), getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[2]", "w").toInt32());

    // tdf#106572 - preventative test matching danger conditions, but imported OK anyway
    CPPUNIT_ASSERT_EQUAL(OUString("First Page"), getProperty<OUString>(getParagraphOrTable(1), "PageDescName"));
}

DECLARE_OOXMLEXPORT_TEST(fdo69656, "Table_cell_auto_width_fdo69656.docx")
{
    // Changed the UT to check "dxa" instead of "auto"
    // For this particular issue file few cells have width type "auto"
    // LO supports VARIABLE and FIXED width type.
    // If type is VARIABLE LO calculates width as percent of PageSize
    // Else if the width is fixed it uses the width value.
    // After changes for fdo76741 the fixed width is exported as "dxa" for DOCX

    // Check for the width type of table and its cells.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblW","type","dxa");
}

DECLARE_OOXMLEXPORT_TEST(testFdo76741, "fdo76741.docx")
{

     // There are two issue related to table in the saved(exported) file
     // - the table alignment in saved file is "left" instead of "center"
     // - the table width type in properties is "auto" instead of "dxa"

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
       return;
    assertXPath(pXmlDoc, "//w:jc", "val", "center");
    assertXPath(pXmlDoc, "//w:tblW", "w", "10081");
    assertXPath(pXmlDoc, "//w:tblW", "type", "dxa");
}

DECLARE_OOXMLEXPORT_TEST(testFdo73541,"fdo73541.docx")
{
    // fdo#73541: The mirrored margins were not imported and mapped correctly in Page Layout
    // Hence <w:mirrorMargins /> tag was not exported back in settings.xml
    xmlDocPtr pXmlDoc = parseExport("word/settings.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:settings/w:mirrorMargins");
}

DECLARE_OOXMLEXPORT_TEST(testFdo106029,"fdo106029.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/settings.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:doNotExpandShiftReturn");
}

DECLARE_OOXMLEXPORT_TEST(testFDO74106, "FDO74106.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:numFmt", "val","hebrew1");
}

DECLARE_OOXMLEXPORT_TEST(testFDO74215, "FDO74215.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    if (!pXmlDoc)
        return;
    // tdf#106849 NumPicBullet xShape should not to be resized.

    // This is dependent on the running system: see MSWordExportBase::BulletDefinitions
    // FIXME: the size of a bullet is defined by GraphicSize property
    // (stored in SvxNumberFormat::aGraphicSize) so use that for the size
    // (properly convert from 100mm to pt (1 inch is 72 pt, 1 pt is 20 twips).

    // On 96 DPI "width:11.25pt;height:11.25pt"; on 120 DPI "width:9pt;height:9pt"
    const OUString sStyle
        = getXPath(pXmlDoc, "/w:numbering/w:numPicBullet[2]/w:pict/v:shape", "style");
    {
        const OUString sWidth = sStyle.getToken(0, ';');
        CPPUNIT_ASSERT(sWidth.startsWith("width:"));
        CPPUNIT_ASSERT(sWidth.endsWith("pt"));
        const double fWidth = sWidth.copy(6, sWidth.getLength() - 8).toDouble();
        const double fXScaleFactor = 96.0 / Application::GetDefaultDevice()->GetDPIX();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.25 * fXScaleFactor, fWidth, 0.001);
    }
    {
        const OUString sHeight = sStyle.getToken(1, ';');
        CPPUNIT_ASSERT(sHeight.startsWith("height:"));
        CPPUNIT_ASSERT(sHeight.endsWith("pt"));
        const double fHeight = sHeight.copy(7, sHeight.getLength() - 9).toDouble();
        const double fYScaleFactor = 96.0 / Application::GetDefaultDevice()->GetDPIY();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.25 * fYScaleFactor, fHeight, 0.001);
    }
}

DECLARE_OOXMLEXPORT_TEST(testColumnBreak_ColumnCountIsZero,"fdo74153.docx")
{
    /* fdo73545: Column Break with Column_count = 0 was not getting preserved.
     * The <w:br w:type="column" /> was missing after roundtrip
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (pXmlDoc)
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:br","type","column");

    //tdf76349 match Word's behavior of treating breaks in single columns as page breaks.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testTdf90697_complexBreaksHeaders,"tdf90697_complexBreaksHeaders.docx")
{
// This is a complex document using many types of section breaks and re-defined headers.
// Paragraphs 44-47 were in two columns
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(45), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

// after that, the section break should switch things back to one column.
    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(50), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());
}

DECLARE_OOXMLEXPORT_TEST(testIndentation, "test_indentation.docx")
{
    // fdo#74141 :There was a problem that in style.xml and document.xml in <w:ind> tag "right" & "left" margin
    // attributes gets added(w:right=0 & w:left=0) if these attributes are not set in original document.
    // This test is to verify <w:ind> does not contain w:right attribute.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:ind", "end", "");
}

DECLARE_OOXMLEXPORT_TEST(testChartInFooter, "chart-in-footer.docx")
{
    // fdo#73872: document contains chart in footer.
    // The problem was that  footer1.xml.rels files for footer1.xml
    // files were missing from docx file after roundtrip.
    xmlDocPtr pXmlDoc = parseExport("word/_rels/footer1.xml.rels");
    if(!pXmlDoc)
        return;

    // Check footer1.xml.rels contains in doc after roundtrip.
    // Check Id = rId1 in footer1.xml.rels
    assertXPath(pXmlDoc,"/rels:Relationships/rels:Relationship","Id","rId1");
    assertXPath(pXmlDoc,
        "/rels:Relationships/rels:Relationship[@Id='rId1']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart");

    xmlDocPtr pXmlDocCT = parseExport("[Content_Types].xml");
    assertXPath(pXmlDocCT,
        "/ContentType:Types/ContentType:Override[@PartName='/word/charts/chart1.xml']",
        "ContentType",
        "application/vnd.openxmlformats-officedocument.drawingml.chart+xml");

    // check the content too
    xmlDocPtr pXmlDocFooter1 = parseExport("word/footer1.xml");
    assertXPath(pXmlDocFooter1,
        "/w:ftr/w:p[1]/w:r/w:drawing/wp:inline/a:graphic/a:graphicData",
        "uri",
        "http://schemas.openxmlformats.org/drawingml/2006/chart");
    assertXPath(pXmlDocFooter1,
        "/w:ftr/w:p[1]/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/c:chart",
        "id",
        "rId1");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    if (xDrawPageSupplier.is())
    {
        // If xDrawPage->getCount()==1, then document contains one shape.
        uno::Reference<container::XIndexAccess> xDrawPage(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount()); // One shape in the doc
    }
}

DECLARE_OOXMLEXPORT_TEST(testNestedTextFrames, "nested-text-frames.odt")
{
    // First problem was LO crashed during export (crash test)

    // Second problem was LO made file corruption, writing out nested text boxes, which can't be handled by Word.
    // Test that all three exported text boxes are anchored to the same paragraph and not each other.
    uno::Reference<text::XTextContent> xTextContent(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange(xTextContent->getAnchor(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());

    xTextContent.set(getShape(2), uno::UNO_QUERY);
    xRange.set(xTextContent->getAnchor(), uno::UNO_QUERY);
    xText.set(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());

    xTextContent.set(getShape(3), uno::UNO_QUERY);
    xRange.set(xTextContent->getAnchor(), uno::UNO_QUERY);
    xText.set(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());
}

DECLARE_OOXMLEXPORT_TEST(testFloatingTablePosition, "floating-table-position.docx")
{
    // Position of shape was wrong, because some conversion was missing.
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    // This was 3295.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5964), getProperty<sal_Int32>(xShape, "HoriOrientPosition"));
    // This was 4611.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8133), getProperty<sal_Int32>(xShape, "VertOrientPosition"));
}

DECLARE_OOXMLEXPORT_TEST(testAbi11739, "abi11739.docx")
{
    // Validation test: order of elements were wrong.
    xmlDocPtr pXmlDoc = parseExport("word/styles.xml");
    if (!pXmlDoc)
        return;
    // Order was: uiPriority, link, basedOn.
    CPPUNIT_ASSERT(getXPathPosition(pXmlDoc, "/w:styles/w:style[3]", "basedOn") < getXPathPosition(pXmlDoc, "/w:styles/w:style[3]", "link"));
    CPPUNIT_ASSERT(getXPathPosition(pXmlDoc, "/w:styles/w:style[3]", "link") < getXPathPosition(pXmlDoc, "/w:styles/w:style[3]", "uiPriority"));
    // Order was: qFormat, unhideWhenUsed.
    CPPUNIT_ASSERT(getXPathPosition(pXmlDoc, "/w:styles/w:style[11]", "unhideWhenUsed") < getXPathPosition(pXmlDoc, "/w:styles/w:style[11]", "qFormat"));
}

DECLARE_OOXMLEXPORT_TEST(testEmbeddedXlsx, "embedded-xlsx.docx")
{
    // check there are two objects and they are FrameShapes
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("FrameShape"), getShape(1)->getShapeType());
    CPPUNIT_ASSERT_EQUAL(OUString("FrameShape"), getShape(2)->getShapeType());

    // check the objects are present in the exported document.xml
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/w:object", 2);

    // finally check the embedded files are present in the zipped document
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
    uno::Sequence<OUString> names = xNameAccess->getElementNames();
    int nSheetFiles = 0;
    int nImageFiles = 0;
    for (int i=0; i<names.getLength(); i++)
    {
        if(names[i].startsWith("word/embeddings/oleObject"))
            nSheetFiles++;
        if(names[i].startsWith("word/media/image"))
            nImageFiles++;
    }
    CPPUNIT_ASSERT_EQUAL(2, nSheetFiles);
    CPPUNIT_ASSERT_EQUAL(2, nImageFiles);
}

DECLARE_OOXMLEXPORT_TEST(testNumberedLists_StartingWithZero, "FDO74105.docx")
{
    /* Issue : Numbered lists Starting with value '0' is not preserved after RT.
     * In numbering.xml, an XML tag <w:start> is optional. If not mentioned,
     * the Numbered list should start from 0.
     * Problem was LO was writing <w:start> for all levels 0-8 with default value "1".
     */
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    if (!pXmlDoc)
      return;

    // Check that we do _not_ export w:start for <w:lvl w:ilvl="0">.
    assertXPath(pXmlDoc, "w:numbering/w:abstractNum[1]/w:lvl[1]/w:start", 0);
}

DECLARE_OOXMLEXPORT_TEST(testPageBreak,"fdo74566.docx")
{
    /*  Break to next page was written into wrong paragraph as <w:pageBreakBefore />.
     *  LO was not preserving Page Break as <w:br w:type="page" />.
     *  Now after fix , LO writes Page Break as the new paragraph and also
     *  preserves the xml tag <w:br>.
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    uno::Reference<text::XTextRange> xParagraph2 = getParagraph(2);
    uno::Reference<text::XTextRange> xParagraph4 = getParagraph(4);

    getRun(xParagraph2, 1, "First Page Second Line");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:br","type","page");
    getRun(xParagraph4, 1, "Second Page First line after Page Break");
}

DECLARE_OOXMLEXPORT_TEST(testOleObject, "test_ole_object.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:object/v:shape/v:imagedata", "o:title", "");
    assertXPath(pXmlDoc,
        "/w:document/w:body/w:p[2]/w:r/w:object/o:OLEObject",
        "DrawAspect",
        "Content");
    // TODO: ProgID="Package" - what is this? Zip with 10k extra header?

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.bin']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject");
    // check the media type too
    xmlDocPtr pXmlDocCT = parseExport("[Content_Types].xml");
    assertXPath(pXmlDocCT,
        "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.bin']",
        "ContentType",
        "application/vnd.openxmlformats-officedocument.oleObject");

}

DECLARE_OOXMLEXPORT_TEST(testFdo74792, "fdo74792.docx")
{
    /*
     * fdo#74792 : The images associated with smart-art data[i].xml
     * were not preserved on exporting to DOCX format
     * Added support to grabbag the rels, with associated images.
     */
    xmlDocPtr pXmlDoc = parseExport("word/diagrams/_rels/data1.xml.rels");
    if(!pXmlDoc)
        return;
    assertXPath(pXmlDoc,"/rels:Relationships/rels:Relationship", 4);
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(
                         comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());

    //check that images are also saved
    OUString const sImageFile( "word/media/OOXDiagramDataRels1_0.jpeg" ); //added anchor id to form a unique name
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName( sImageFile ), uno::UNO_QUERY);
    CPPUNIT_ASSERT( xInputStream.is() );
}

DECLARE_OOXMLEXPORT_TEST(testFdo77718, "fdo77718.docx")
{
    //in case of multiple smart arts the names for images were getting
    //repeated and thereby causing a data loss as the binary stream was
    //getting over written. This test case ensures that unique names are
    //given for images in different smart arts.
    xmlDocPtr pXmlDataRels1 = parseExport("word/diagrams/_rels/data1.xml.rels");
    if( !pXmlDataRels1 )
        return;

    xmlDocPtr pXmlDataRels2 = parseExport("word/diagrams/_rels/data2.xml.rels");
    if( !pXmlDataRels2 )
        return;

    //ensure that the rels file is present.
    assertXPath(pXmlDataRels1,"/rels:Relationships/rels:Relationship", 4);
    assertXPath(pXmlDataRels2,"/rels:Relationships/rels:Relationship", 4);

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(
                         comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());

    //check that images are also saved
    OUString const sImageFile1( "word/media/OOXDiagramDataRels1_0.jpeg" ); //added anchor id to form a unique name
    uno::Reference<io::XInputStream> xInputStream1(xNameAccess->getByName( sImageFile1 ), uno::UNO_QUERY);
    CPPUNIT_ASSERT( xInputStream1.is() );

    //check that images are saved for other smart-arts as well.
    OUString const sImageFile2( "word/media/OOXDiagramDataRels2_0.jpeg" ); //added anchor id to form a unique name
    uno::Reference<io::XInputStream> xInputStream2(xNameAccess->getByName( sImageFile2 ), uno::UNO_QUERY);
    CPPUNIT_ASSERT( xInputStream2.is() );
}

DECLARE_OOXMLEXPORT_TEST(testTableCurruption, "tableCurrupt.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/header2.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(pXmlDoc) ;
    assertXPath(pXmlDoc, "/w:hdr/w:tbl[1]/w:tr[1]/w:tc[1]",1);

    // tdf#116549: header paragraph should not have a bottom border.
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("First Page"), "HeaderText");
    table::BorderLine2 aHeaderBottomBorder = getProperty<table::BorderLine2>( getParagraphOfText( 1, xHeaderText ), "BottomBorder");
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), aHeaderBottomBorder.LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testDateControl, "date-control.docx")
{
    // check XML
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:date", "fullDate", "2014-03-05T00:00:00Z");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:date/w:dateFormat", "val", "dddd, dd' de 'MMMM' de 'yyyy");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:date/w:lid", "val", "es-ES");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r/w:t", u"mi\u00E9rcoles, 05 de marzo de 2014");

    // check imported control
    uno::Reference<drawing::XControlShape> xControl(getShape(1), uno::UNO_QUERY);
    util::Date aDate = getProperty<util::Date>(xControl->getControl(), "Date");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5),    sal_Int32(aDate.Day));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),    sal_Int32(aDate.Month));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2014), sal_Int32(aDate.Year));
}

DECLARE_OOXMLEXPORT_TEST(test_OpeningBrace, "2120112713_OpenBrace.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Checking for OpeningBrace tag
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/m:oMath[1]/m:d[1]/m:dPr[1]/m:begChr[1]","val","");
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
DECLARE_OOXMLEXPORT_TEST(testTdf38778, "tdf38778_properties_in_run_for_field.doc")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    const OUString psz("20");
    const OUString pszCs("20");

    // w:fldCharType="begin"
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w:sz",   "val", psz);
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w:szCs", "val", pszCs);

    // PAGE
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[4]/w:rPr/w:sz",   "val", psz);
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[4]/w:rPr/w:szCs", "val", pszCs);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[4]/w:instrText",  " PAGE ");

    // w:fldCharType="separate"
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[5]/w:rPr/w:sz",   "val", psz);
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[5]/w:rPr/w:szCs", "val", pszCs);

    // field result: 1
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[6]/w:rPr/w:sz",   "val", psz);
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[6]/w:rPr/w:szCs", "val", pszCs);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[6]/w:t",          "1"); // field result

    // w:fldCharType="end"
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[7]/w:rPr/w:sz",   "val", psz);
    assertXPath(pXmlDoc,        "/w:document/w:body/w:p[1]/w:r[7]/w:rPr/w:szCs", "val", pszCs);
}

DECLARE_OOXMLEXPORT_TEST(testFDO76312, "FDO76312.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]");
}

DECLARE_OOXMLEXPORT_TEST(testComboBoxControl, "combobox-control.docx")
{
    // check XML
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dropDownList/w:listItem[1]", "value", "manolo");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dropDownList/w:listItem[2]", "value", "pepito");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r/w:t", "Manolo");

    // check imported control
    uno::Reference<drawing::XControlShape> xControl(getShape(1), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(OUString("Manolo"), getProperty<OUString>(xControl->getControl(), "Text"));

    uno::Sequence<OUString> aItems = getProperty< uno::Sequence<OUString> >(xControl->getControl(), "StringItemList");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aItems.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("manolo"), aItems[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("pepito"), aItems[1]);
}

DECLARE_OOXMLEXPORT_TEST(testCheckBoxControl, "checkbox-control.docx")
{
    // check XML
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w14:checkbox/w14:checked", "val", "1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w14:checkbox/w14:checkedState", "val", "2612");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w14:checkbox/w14:uncheckedState", "val", "2610");

    // TODO: import control and add a check here
}

DECLARE_OOXMLEXPORT_TEST(testParagraphWithComments, "paragraphWithComments.docx")
{
    /* Comment id's were getting overwritten for annotation mark(s),
       which was causing a mismatch in the relationship for comment id's
       in document.xml and comment.xml
    */
    xmlDocPtr pXmlDoc  = parseExport("word/document.xml");
    xmlDocPtr pXmlComm = parseExport("word/comments.xml");
    if(!pXmlDoc)
        return;

    sal_Int32 idInDocXml     = getXPath(pXmlDoc,"/w:document/w:body/w:p[3]/w:commentRangeEnd[1]","id").toInt32();
    sal_Int32 idInCommentXml = getXPath(pXmlComm,"/w:comments/w:comment[1]","id").toInt32();
    CPPUNIT_ASSERT_EQUAL( idInDocXml, idInCommentXml );
}

DECLARE_OOXMLEXPORT_TEST(testTdf104707_urlComment, "tdf104707_urlComment.odt")
{
    xmlDocPtr pXmlComm = parseExport("word/comments.xml");
    CPPUNIT_ASSERT(pXmlComm);
    CPPUNIT_ASSERT_EQUAL( OUString("https://bugs.documentfoundation.org/show_bug.cgi?id=104707"), getXPathContent(pXmlComm,"/w:comments/w:comment/w:p/w:hyperlink/w:r/w:t") );
}

DECLARE_OOXMLEXPORT_TEST(testOLEObjectinHeader, "2129393649.docx")
{
    // fdo#76015 : Document contains oleobject in header xml.
    // Problem was relationship entry for oleobject from header was
    // exported into document.xml.rels file because of this rels file
    // for headers were missing from document/word/rels.
    xmlDocPtr pXmlDoc = parseExport("word/_rels/header1.xml.rels");
    if(!pXmlDoc)
        return;

    assertXPath(pXmlDoc,"/rels:Relationships/rels:Relationship[1]","Id","rId1");

    xmlDocPtr pXmlDocCT = parseExport("[Content_Types].xml");

    // check the media type too
    assertXPath(pXmlDocCT,
        "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.bin']",
        "ContentType",
        "application/vnd.openxmlformats-officedocument.oleObject");

    // check the content too
    xmlDocPtr pXmlDocHeader1 = parseExport("word/header1.xml");
    assertXPath(pXmlDocHeader1,
        "/w:hdr/w:tbl/w:tr[1]/w:tc[2]/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "Word.Picture.8");
    xmlDocPtr pXmlDocHeader2 = parseExport("word/header2.xml");
    assertXPath(pXmlDocHeader2,
        "/w:hdr/w:tbl/w:tr[1]/w:tc[2]/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "Word.Picture.8");
}

DECLARE_OOXMLEXPORT_TEST(test_ClosingBrace, "2120112713.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Checking for ClosingBrace tag
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/m:oMath[1]/m:d[2]/m:dPr[1]/m:endChr[1]","val","");
}

DECLARE_OOXMLEXPORT_TEST(testlvlPicBulletId, "lvlPicBulletId.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    if (!pXmlDoc)
       return;
    assertXPath(pXmlDoc, "/w:numbering[1]/w:abstractNum[1]/w:lvl[1]/w:lvlPicBulletId[1]", 0);
}

DECLARE_OOXMLEXPORT_TEST(testSdtContent, "SdtContent.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
       return;
    assertXPath(pXmlDoc, "/w:hdr[1]/w:sdt[1]/w:sdtContent[1]/w:p[1]/w:del[1]");
}

#if 0
// Currently LibreOffice exports custom geometry for this up arrow, not preset shape.
// When LibreOffice can export preset shape with correct modifiers, then this test can be re-enabled.

DECLARE_OOXMLEXPORT_TEST(testFdo76016, "fdo76016.docx")
{
    // check XML
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "//a:graphic/a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]", "name", "adj1");
    assertXPath(pXmlDoc, "//a:graphic/a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]", "name", "adj2");
}
#endif

DECLARE_OOXMLEXPORT_TEST(testFileWithInvalidImageLink, "FileWithInvalidImageLink.docx")
{
    /* In case if the original file has an image whose link is
       invalid, then the RT file used to result in corruption
       since the exported image would be an empty image.
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
      return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/pic:pic[1]/pic:blipFill[1]/a:blip[1]", "embed", "");
}

DECLARE_OOXMLEXPORT_TEST(testContentTypeDOCX, "fdo80410.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc) // only test the export, not initial import
       return;

    assertXPath(pXmlDoc,
        "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.docx']",
        "ContentType",
        "application/vnd.openxmlformats-officedocument.wordprocessingml.document");
    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.docx']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[6]/w:r/w:object/o:OLEObject",
        "ProgID",
        "Word.Document.12");
}

DECLARE_OOXMLEXPORT_TEST(testContentTypeXLSM, "fdo76098.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc, "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/Microsoft_Excel_Macro-Enabled_Worksheet1.xlsm']", "ContentType", "application/vnd.ms-excel.sheet.macroEnabled.12");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/charts/_rels/chart1.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='../embeddings/Microsoft_Excel_Macro-Enabled_Worksheet1.xlsm']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocChart1 = parseExport("word/charts/chart1.xml");
    assertXPath(pXmlDocChart1,
        "/c:chartSpace/c:externalData",
        "id",
        "rId1");
}

DECLARE_OOXMLEXPORT_TEST(test76108, "test76108.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc) return;
    //docx file after RT is getting corrupted.
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/w:fldChar[1]", "fldCharType", "begin");
}

DECLARE_OOXMLEXPORT_TEST(testTCTagMisMatch, "TCTagMisMatch.docx")
{
   // TCTagMisMatch.docx : This document contains a empty table with borders.
   // there was a TC tag mismatch which resulted into a crash.

   xmlDocPtr pXmlDoc = parseExport("word/document.xml");
   if(!pXmlDoc)
      return;
   assertXPath(pXmlDoc,"/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl[1]/w:tr[1]/w:tc[1]",0);
   assertXPath(pXmlDoc,"/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]", 1);
}

DECLARE_OOXMLEXPORT_TEST(testFDO78292, "FDO78292.docx")
{
   //text node is a leaf node, it should not have any children
   xmlDocPtr pXmlDoc = parseExport("word/document.xml");
   if(!pXmlDoc)
      return;
   assertXPath(pXmlDoc,"/w:document/w:body/w:p[14]/w:sdt[3]/w:sdtPr[1]/w:text/w14:checked",0);
}

DECLARE_OOXMLEXPORT_TEST(testSimpleSdts, "simple-sdts.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:text", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:id", 3);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:picture", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:group", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:citation", 1);
}

DECLARE_OOXMLEXPORT_TEST(testEmbeddedExcelChart, "EmbeddedExcelChart.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc) // only test the export, not initial import
       return;

    assertXPath(pXmlDoc,
        "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.xls']",
        "ContentType",
        "application/vnd.ms-excel");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xls']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject");

    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p/w:r/w:object/o:OLEObject",
        "ProgID",
        "Excel.Chart.8");
}

DECLARE_OOXMLEXPORT_TEST(testTdf83227, "tdf83227.docx")
{
    // Bug document contains a rotated image, which is handled as a draw shape (not as a Writer image) on export.
    if (!mbExported)
        return;

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("word/media/image1.png")));
    // This was also true, image was written twice.
    CPPUNIT_ASSERT_EQUAL(false, bool(xNameAccess->hasByName("word/media/image2.png")));
}

DECLARE_OOXMLEXPORT_TEST(testTdf103001, "tdf103001.docx")
{
    // The same image is featured in the header and in the body text, make sure
    // the header relation is still written, even when caching is enabled.
    if (!mbExported)
        return;

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
    // This failed: header reused the RelId of the body text, even if RelIds
    // are local to their stream.
    CPPUNIT_ASSERT(xNameAccess->hasByName("word/_rels/header1.xml.rels"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf92521, "tdf92521.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        // There should be a section break that's in the middle of the document: right after the table.
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:sectPr", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf102466, "tdf102466.docx")
{
    // the problem was: file is truncated: the first page is missing.
    // More precisely, the table in the first page was clipped.
    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        sal_Int32 nFlyPrtHeight = getXPath(pXmlDoc, "(/root/page[1]//fly)[1]/infos/prtBounds", "height").toInt32();
        sal_Int32 nTableHeight = getXPath(pXmlDoc, "(/root/page[1]//fly)[1]/tab/infos/bounds", "height").toInt32();
        CPPUNIT_ASSERT_MESSAGE("The table is clipped in a fly frame.", nFlyPrtHeight >= nTableHeight);
    }

    // check how much pages we have
    CPPUNIT_ASSERT_EQUAL(10, getPages());

    // check content of the first page
    {
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

        // no border
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xFrame, "LineWidth"));
    }

    // Make sure we have 19 tables created
    {
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(19), xTables->getCount( ));

        // check the text inside first cell of the first table
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);

        const OUString aMustHaveText = "Requerimientos del  Cliente";
        const OUString aActualText   = xCell->getString();

        CPPUNIT_ASSERT(aActualText.indexOf(aMustHaveText) > 0);
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf99090_pgbrkAfterTable, "tdf99090_pgbrkAfterTable.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        // There should be a regular page break that's in the middle of the document: right after the table.
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:br", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf96750_landscapeFollow, "tdf96750_landscapeFollow.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStyle, "IsLandscape"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf86926_A3, "tdf86926_A3.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(42000), getProperty<sal_Int32>(xStyle, "Height"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(29700), getProperty<sal_Int32>(xStyle, "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf64372_continuousBreaks,"tdf64372_continuousBreaks.docx")
{
    //There are no page breaks, so everything should be on the first page.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testTdf92724_continuousBreaksComplex,"tdf92724_continuousBreaksComplex.docx")
{
    //There are 2 page breaks, so there should be 3 pages.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3), xCursor->getPage());
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
        OUString sPageStyleName = getProperty<OUString>( xCursor, "PageStyleName" );
        uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(sPageStyleName), "HeaderText");
// Specific case to avoid.  Testing separately (even though redundant).
// The first header (defined on page 3) ONLY is shown IF the section happens to start on a new page (which it doesn't in this document).
        CPPUNIT_ASSERT( xHeaderText->getString() != "Third section - First page header. No follow defined" );
// Same test stated differently: Pages 4 and 5 OUGHT to use "Second section header", but currently don't.  Page 6 does.
        if( nPages <= 3 )
            CPPUNIT_ASSERT_EQUAL( OUString("First section header"), xHeaderText->getString() );
        else
            CPPUNIT_ASSERT( xHeaderText->getString() == "First section header" || xHeaderText->getString() == "Second section header" );

        xCursor->jumpToPage( --nPages );
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf95367_inheritFollowStyle, "tdf95367_inheritFollowStyle.docx")
{
    CPPUNIT_ASSERT_EQUAL(OUString("header"),  parseDump("/root/page[2]/header/txt/text()"));
}

DECLARE_OOXMLEXPORT_TEST(testInheritFirstHeader,"inheritFirstHeader.docx")
{
// First page headers always link to last used first header, never to a follow header
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);

    xCursor->jumpToLastPage();
    OUString sPageStyleName = getProperty<OUString>( xCursor, "PageStyleName" );
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(sPageStyleName), "HeaderText");
    CPPUNIT_ASSERT_EQUAL( OUString("Last Header"), xHeaderText->getString() );

    xCursor->jumpToPreviousPage();
    sPageStyleName = getProperty<OUString>( xCursor, "PageStyleName" );
    xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(sPageStyleName), "HeaderText");
    CPPUNIT_ASSERT_EQUAL( OUString("First Header"), xHeaderText->getString() );

    xCursor->jumpToPreviousPage();
    sPageStyleName = getProperty<OUString>( xCursor, "PageStyleName" );
    xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(sPageStyleName), "HeaderText");
    CPPUNIT_ASSERT_EQUAL( OUString("Follow Header"), xHeaderText->getString() );
}

#if HAVE_MORE_FONTS
DECLARE_OOXMLEXPORT_TEST(testTdf81345_045Original,"tdf81345.docx")
{
    //Header wasn't replaced  and columns were missing because no new style was created.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);

    xCursor->jumpToPage(2);
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    CPPUNIT_ASSERT(pageStyleName != "Standard");

    // tdf89297 Styles were being added before their base/parent/inherited-from style existed, and so were using default settings.
    uno::Reference<container::XNameAccess> xParaStyles(getStyles("ParagraphStyles"));
    uno::Reference<beans::XPropertySet> xStyle(xParaStyles->getByName("Pull quote"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6736947), getProperty<sal_Int32>(xStyle, "CharColor"));
}
#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
