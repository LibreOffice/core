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
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <string>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const SAL_OVERRIDE {
        const char* aBlacklist[] = {
            "math-escape.docx",
            "math-mso2k7.docx",
            "ImageCrop.docx",
            "test_GIF_ImageCrop.docx",
            "test_PNG_ImageCrop.docx"
        };
        std::vector<const char*> vBlacklist(aBlacklist, aBlacklist + SAL_N_ELEMENTS(aBlacklist));

        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".docx") && std::find(vBlacklist.begin(), vBlacklist.end(), filename) == vBlacklist.end());
    }

    /**
     * Validation handling
     */
    bool mustValidate(const char* filename) const SAL_OVERRIDE
    {
        const char* aWhitelist[] = {
            "paragraph-mark-nonempty.odt"
        };
        std::vector<const char*> vWhitelist(aWhitelist, aWhitelist + SAL_N_ELEMENTS(aWhitelist));

        return std::find(vWhitelist.begin(), vWhitelist.end(), filename) != vWhitelist.end();
    }
protected:
    bool CjkNumberedListTestHelper(sal_Int16 &nValue)
    {
        bool isNumber = false;
        uno::Reference<text::XTextRange> xPara(getParagraph(1));
        uno::Reference< beans::XPropertySet > properties( xPara, uno::UNO_QUERY);
        properties->getPropertyValue("NumberingIsNumber") >>= isNumber;
        if (!isNumber)
            return false;
        uno::Reference<container::XIndexAccess> xLevels( properties->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
        uno::Sequence< beans::PropertyValue > aPropertyValue;
        xLevels->getByIndex(0) >>= aPropertyValue;
        for( int j = 0 ; j< aPropertyValue.getLength() ; ++j)
        {
            beans::PropertyValue aProp= aPropertyValue[j];
            if (aProp.Name == "NumberingType")
            {
                nValue = aProp.Value.get<sal_Int16>();
                return true;
            }
        }
        return false;

    }
};

DECLARE_OOXMLEXPORT_TEST(testfdo81381, "fdo81381.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:object[1]/o:OLEObject[1]", "DrawAspect", "Icon");
}

DECLARE_OOXMLEXPORT_TEST(testSdtAlias, "sdt-alias.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    // <w:alias> was completely missing.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:alias", "val", "Subtitle");
}

DECLARE_OOXMLEXPORT_TEST(testSdtDateCharformat, "sdt-date-charformat.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
    {
        // character formatting (bold) was missing, this was 0
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r/w:rPr/w:b", 1);
        // alias was also missing.
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:alias", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testFooterBodyDistance, "footer-body-distance.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
        // Page break was exported as section break, this was 0
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:br", 1);
}

DECLARE_OOXMLEXPORT_TEST(testfdo81031, "fdo81031.docx")
{
    // vml image was not rendered
    // As there are also numPicBullets in the file,
    // the fragmentPath was not changed hence relationships were not resolved.

    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> xImage(image, uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic = getProperty<uno::Reference<graphic::XGraphic> >(xImage, "Graphic");
    uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(381), xBitmap->getSize().Width );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(148), xBitmap->getSize().Height );
}

DECLARE_OOXMLEXPORT_TEST(testPlausableBorder, "plausable-border.docx")
{
    // sw::util::IsPlausableSingleWordSection() did not merge two page styles due to borders.
    if (xmlDocPtr pXmlDoc = parseExport())
        // Page break was exported as section break, this was 0
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:br", 1);
}

DECLARE_OOXMLEXPORT_TEST(testUnwantedSectionBreak, "unwanted-section-break.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
        // This was 2: an additional sectPr was added to the document.
        assertXPath(pXmlDoc, "//w:sectPr", 1);
}

DECLARE_OOXMLEXPORT_TEST(testfdo80897 , "fdo80897.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:bodyPr/a:prstTxWarp", "prst", "textTriangle");
}



DECLARE_OOXMLEXPORT_TEST(testFdo80997, "fdo80997.docx")
{
    // The problem was that the DOCX exporter not able to export text behind textbox, if textbox has a wrap property.
    uno::Reference< text::XTextRange > xParagraph = getParagraph( 1 );
    uno::Reference< text::XTextRange > xText = getRun( xParagraph, 1, " text");
}

DECLARE_OOXMLEXPORT_TEST(testFdo80902, "fdo80902.docx")
{
    // The problem was that the docGrid type was set as default so fix it for other grid type
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:docGrid", "type", "lines");
}

DECLARE_OOXMLEXPORT_TEST(testParaShading, "para-shading.docx")
{
    // Make sure the themeColor attribute is not written when it would be empty.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:shd", "themeColor");
}

DECLARE_OOXMLEXPORT_TEST(testFirstHeaderFooter, "first-header-footer.docx")
{
    // Test import and export of a section's headerf/footerf properties.
    // (copied from a ww8export test, with doc converted to docx using Word)

    // The document has 6 pages. Note that we don't test if 4 or just 2 page
    // styles are created, the point is that layout should be correct.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer"),  parseDump("/root/page[1]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header"),   parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer"),   parseDump("/root/page[2]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header"),  parseDump("/root/page[3]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer"),  parseDump("/root/page[3]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page header2"), parseDump("/root/page[4]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer 2"), parseDump("/root/page[4]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header 2"), parseDump("/root/page[5]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer 2"), parseDump("/root/page[5]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header 2"),  parseDump("/root/page[6]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer 2"),  parseDump("/root/page[6]/footer/txt/text()"));
}

DECLARE_OOXMLEXPORT_TEST(testFDO83044, "fdo83044.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:text", 1);
}

DECLARE_OOXMLEXPORT_TEST(testfdo83428, "fdo83428.docx")
{
     uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
     uno::Reference<document::XDocumentProperties> xProps(xDocumentPropertiesSupplier->getDocumentProperties());
     uno::Reference<beans::XPropertySet> xUDProps(xProps->getUserDefinedProperties(), uno::UNO_QUERY);
     CPPUNIT_ASSERT_EQUAL(OUString("Document"), getProperty<OUString>(xUDProps, "Testing"));
}

DECLARE_OOXMLEXPORT_TEST(testShapeInFloattable, "shape-in-floattable.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // No nested drawingML w:txbxContent.
        assertXPath(pXmlDoc, "//mc:Choice//w:txbxContent//w:txbxContent", 0);
        // Instead, make sure we have a separate shape and group shape:
        assertXPath(pXmlDoc, "//mc:AlternateContent//mc:Choice[@Requires='wps']", 1);
        assertXPath(pXmlDoc, "//mc:AlternateContent//mc:Choice[@Requires='wpg']", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testEmptyAnnotationMark, "empty-annotation-mark.docx")
{
    if (mbExported)
    {
        // Delete the word that is commented, and save again.
        uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 3);
        CPPUNIT_ASSERT_EQUAL(OUString("with"), xRun->getString());
        xRun->setString("");
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        xStorable->store();

        // Then inspect the OOXML markup of the modified document model.
        xmlDocPtr pXmlDoc = parseExport("word/document.xml");
        // There were two commentReference nodes.
        assertXPath(pXmlDoc, "//w:commentReference", "id", "0");
        // Empty comment range was not ignored on export, this was 1.
        assertXPath(pXmlDoc, "//w:commentRangeStart", 0);
        // Ditto.
        assertXPath(pXmlDoc, "//w:commentRangeEnd", 0);
    }
}

DECLARE_OOXMLEXPORT_TEST(testDropdownInCell, "dropdown-in-cell.docx")
{
    // First problem: table was missing from the document, this was 0.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // Second problem: dropdown shape wasn't anchored inside the B1 cell.
    uno::Reference<text::XTextContent> xShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xAnchor = xShape->getAnchor();
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRangeCompare> xTextRangeCompare(xCell, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextRangeCompare->compareRegionStarts(xAnchor, xCell));
}

DECLARE_OOXMLEXPORT_TEST(testTableAlignment, "table-alignment.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was LEFT_AND_WIDTH, i.e. table alignment wasn't imported correctly.
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::RIGHT, getProperty<sal_Int16>(xTable, "HoriOrient"));
}

DECLARE_OOXMLEXPORT_TEST(testSdtIgnoredFooter, "sdt-ignored-footer.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // This was 1, make sure no w:sdt sneaks into the main document from the footer.
        assertXPath(pXmlDoc, "//w:sdt", 0);
    }
}

DECLARE_OOXMLEXPORT_TEST(testSdtRunPicture, "sdt-run-picture.docx")
{
    // SDT around run was exported as SDT around paragraph
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // This was 1: there was an SDT around w:p.
        assertXPath(pXmlDoc, "//w:body/w:sdt", 0);
        // This was 0: there were no SDT around w:r.
        assertXPath(pXmlDoc, "//w:body/w:p/w:sdt", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testChartDupe, "chart-dupe.docx")
{
    // Single chart was exported back as two charts.
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    // This was 2, on second import we got a duplicated chart copy.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xEmbeddedObjects->getCount());
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
    uno::Reference<beans::XPropertySet> xStyle(getStyles("CharacterStyles")->getByName("ListLabel 1"), uno::UNO_QUERY);
    // This was Calibri, i.e. custom font of the numbering itself ("1.\t") was lost on import.
    CPPUNIT_ASSERT_EQUAL(OUString("Verdana"), getProperty<OUString>(xStyle, "CharFontName"));
}

DECLARE_OOXMLEXPORT_TEST(testDrawingmlFlipv, "drawingml-flipv.docx")
{
    // The problem was that the shape had vertical flip only, but then we added rotation as well on export.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPathNoAttribute(pXmlDoc, "//a:xfrm", "rot");
}

DECLARE_OOXMLEXPORT_TEST(testRot90Fliph, "rot90-fliph.docx")
{
    // The problem was that a shape rotation of 90° got turned into 270° after roundtrip.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        assertXPath(pXmlDoc, "//a:xfrm", "flipH", "1");
        // This was 16200000 (270 * 60000).
        assertXPath(pXmlDoc, "//a:xfrm", "rot", "5400000");
    }
}

DECLARE_OOXMLEXPORT_TEST(testRot180Flipv, "rot180-flipv.docx")
{
    // 180° rotation got lost after roundtrip.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        assertXPath(pXmlDoc, "//a:xfrm", "flipV", "1");
        // This attribute was completely missing.
        assertXPath(pXmlDoc, "//a:xfrm", "rot", "10800000");
    }
}

DECLARE_OOXMLEXPORT_TEST(testRot270Flipv, "rot270-flipv.docx")
{
    // 270° rotation got turned into 90° after roundtrip.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        assertXPath(pXmlDoc, "//a:xfrm", "flipV", "1");
        // This was 5400000.
        assertXPath(pXmlDoc, "//a:xfrm", "rot", "16200000");
    }
}

DECLARE_OOXMLEXPORT_TEST(testMsoPosition, "bnc884615-mso-position.docx")
{
    if(xmlDocPtr doc = parseExport("word/footer1.xml"))
    {
        // We write the frames out in different order than they were read, so check it's the correct
        // textbox first by checking width. These tests may need reordering if that gets fixed.
        OUString style1 = getXPath(doc, "/w:ftr/w:p/w:r[3]/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "style");
        CPPUNIT_ASSERT( style1.indexOf( ";width:531pt;" ) >= 0 );
        CPPUNIT_ASSERT( style1.indexOf( ";mso-position-vertical-relative:page" ) >= 0 );
        CPPUNIT_ASSERT( style1.indexOf( ";mso-position-horizontal-relative:page" ) >= 0 );
        OUString style2 = getXPath(doc, "/w:ftr/w:p/w:r[4]/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "style");
        CPPUNIT_ASSERT( style2.indexOf( ";width:549pt;" ) >= 0 );
        CPPUNIT_ASSERT( style2.indexOf( ";mso-position-vertical-relative:text" ) >= 0 );
        CPPUNIT_ASSERT( style2.indexOf( ";mso-position-horizontal:center" ) >= 0 );
        CPPUNIT_ASSERT( style2.indexOf( ";mso-position-horizontal-relative:text" ) >= 0 );
        OUString style3 = getXPath(doc, "/w:ftr/w:p/w:r[5]/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "style");
        CPPUNIT_ASSERT( style3.indexOf( ";width:36pt;" ) >= 0 );
        CPPUNIT_ASSERT( style3.indexOf( ";mso-position-horizontal-relative:text" ) >= 0 );
        CPPUNIT_ASSERT( style3.indexOf( ";mso-position-vertical-relative:text" ) >= 0 );
    }
    if(xmlDocPtr doc = parseExport("word/header1.xml"))
    {
        OUString style1 = getXPath(doc, "/w:hdr/w:p/w:r[2]/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "style");
        CPPUNIT_ASSERT( style1.indexOf( ";width:335.75pt;" ) >= 0 );
        CPPUNIT_ASSERT( style1.indexOf( ";mso-position-horizontal-relative:page" ) >= 0 );
        CPPUNIT_ASSERT( style1.indexOf( ";mso-position-vertical-relative:page" ) >= 0 );
        OUString style2 = getXPath(doc, "/w:hdr/w:p/w:r[3]/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "style");
        CPPUNIT_ASSERT( style2.indexOf( ";width:138.15pt;" ) >= 0 );
        CPPUNIT_ASSERT( style2.indexOf( ";mso-position-horizontal-relative:page" ) >= 0 );
        CPPUNIT_ASSERT( style2.indexOf( ";mso-position-vertical-relative:page" ) >= 0 );
        OUString style3 = getXPath(doc, "/w:hdr/w:p/w:r[4]/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "style");
        CPPUNIT_ASSERT( style3.indexOf( ";width:163.8pt;" ) >= 0 );
        CPPUNIT_ASSERT( style3.indexOf( ";mso-position-horizontal-relative:page" ) >= 0 );
        CPPUNIT_ASSERT( style3.indexOf( ";mso-position-vertical-relative:page" ) >= 0 );
    }
}

DECLARE_OOXMLEXPORT_TEST(testWpsCharColor, "wps-char-color.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was -1, i.e. the character color was default (-1), not white.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xffffff), getProperty<sal_Int32>(xShape->getStart(), "CharColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTableStyleCellBackColor, "table-style-cell-back-color.docx")
{
    // The problem was that cell background was white, not green.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    // This was 0xffffff.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00ff00), getProperty<sal_Int32>(xCell, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTableStyleBorder, "table-style-border.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // This was 0, the second cell was missing its right border.
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A2");
    CPPUNIT_ASSERT(getProperty<table::BorderLine2>(xCell, "RightBorder").LineWidth > 0);

    // This was also 0 (even after fixing the previous problem), the first cell was missing its right border, too.
    xCell = xTable->getCellByName("A1");
    CPPUNIT_ASSERT(getProperty<table::BorderLine2>(xCell, "RightBorder").LineWidth > 0);
}

DECLARE_OOXMLEXPORT_TEST(testTableStyleBorderExport, "table-style-border-export.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A3");
    // Bottom border was white, so this was 0xffffff.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x8064A2), getProperty<table::BorderLine2>(xCell, "BottomBorder").Color);
}

DECLARE_OOXMLEXPORT_TEST(testAnchorPosition, "anchor-position.docx")
{
    // The problem was that the at-char anchored picture was at the end of the
    // paragraph, so there were only two postions: a Text, then a Frame one.
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), getProperty<OUString>(getRun(getParagraph(1), 1), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"), getProperty<OUString>(getRun(getParagraph(1), 2), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), getProperty<OUString>(getRun(getParagraph(1), 3), "TextPortionType"));
}

DECLARE_OOXMLEXPORT_TEST(testMultiPageToc, "multi-page-toc.docx")
{
    // Import of this document triggered an STL assertion.

    // Document has a ToC from its second paragraph.
    uno::Reference<container::XNamed> xTextSection = getProperty< uno::Reference<container::XNamed> >(getParagraph(2), "TextSection");
    CPPUNIT_ASSERT_EQUAL(OUString("Table of Contents1"), xTextSection->getName());
    // There should be a field in the header as well.
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "HeaderText");
    CPPUNIT_ASSERT_EQUAL(OUString("TextFieldStart"), getProperty<OUString>(getRun(getParagraphOfText(1, xHeaderText), 1), "TextPortionType"));
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

DECLARE_OOXMLEXPORT_TEST(testCropPixel, "crop-pixel.docx")
{
    // If map mode of the graphic is in pixels, then we used to handle original
    // size of the graphic as mm100, but it was in pixels.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // This is 17667 in the original document, was 504666 (so the image
        // become invisible), now is around 19072.
        (void) pXmlDoc;
        CPPUNIT_ASSERT(getXPath(pXmlDoc, "//a:srcRect", "l").toInt32() <= 21856);
    }
}

DECLARE_OOXMLEXPORT_TEST(testEffectExtent, "effect-extent.docx")
{
    // The problem was that in case there were no shadows on the picture, we
    // wrote a <wp:effectExtent> full or zeros.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        // E.g. this was 0.
        assertXPath(pXmlDoc, "//wp:effectExtent", "l", "114300");
}

DECLARE_OOXMLEXPORT_TEST(testEm, "em.docx")
{
    // Test all possible <w:em> arguments.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::NONE, getProperty<sal_Int16>(getRun(getParagraph(1), 1), "CharEmphasis"));
    // This was ACCENT_ABOVE.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::DOT_ABOVE, getProperty<sal_Int16>(getRun(getParagraph(1), 2), "CharEmphasis"));
    // This was DOT_ABOVE.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::ACCENT_ABOVE, getProperty<sal_Int16>(getRun(getParagraph(1), 3), "CharEmphasis"));
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::CIRCLE_ABOVE, getProperty<sal_Int16>(getRun(getParagraph(1), 4), "CharEmphasis"));
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::DOT_BELOW, getProperty<sal_Int16>(getRun(getParagraph(1), 5), "CharEmphasis"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo77716, "fdo77716.docx")
{
    // The problem was that there should be 200 twips spacing between the two paragraphs, but there wasn't any.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("ParagraphStyles")->getByName("Standard"), uno::UNO_QUERY);
    // This was 0.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(200)), getProperty<sal_Int32>(xStyle, "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testAfterlines, "afterlines.docx")
{
    // This was 353, i.e. the value of <w:spacing w:after="200"> from <w:pPrDefault>, instead of <w:spacing w:afterLines="100"/> from <w:pPr>.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(423), getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testParagraphMark, "paragraph-mark.docx")
{
    // The problem was that we didn't handle the situation when an empty paragraph's marker had both a char style and some direct formatting.

    // This was 11.
    CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(getParagraph(1), "CharHeight"));
    // This was empty.
    CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(getRun(getParagraph(1), 1), "CharStyleName"));
}

DECLARE_OOXMLEXPORT_TEST(testParagraphMarkNonempty, "paragraph-mark-nonempty.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport())
        // There were two <w:sz> elements, make sure the 40 one is is dropped and the 20 one is kept.
        assertXPath(pXmlDoc, "//w:p/w:pPr/w:rPr/w:sz", "val", "20");
}

DECLARE_OOXMLEXPORT_TEST(testPageBreakBefore, "page-break-before.docx")
{
    // This was style::BreakType_PAGE_BEFORE, i.e. page break wasn't ignored, as it should have been.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE, getProperty<style::BreakType>(getParagraph(2), "BreakType"));
}

DECLARE_OOXMLEXPORT_TEST(testTableRtl, "table-rtl.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was text::WritingMode2::LR_TB, i.e. direction of the table was ignored.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>(xTable, "WritingMode"));
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlCjklist30, "cjklist30.docx")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::TIAN_GAN_ZH, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlCjklist31, "cjklist31.docx")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::DI_ZI_ZH, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlCjklist34, "cjklist34.docx")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_UPPER_ZH_TW, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlCjklist35, "cjklist35.docx")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_LOWER_ZH, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testNumParentStyle, "num-parent-style.docx")
{
    // This was "Outline", i.e. <w:numId> was not imported from the Heading 2 paragraph style.
    CPPUNIT_ASSERT(getProperty<OUString>(getParagraph(4), "NumberingStyleName").startsWith("WWNum"));
}

DECLARE_OOXMLEXPORT_TEST(testNumOverrideLvltext, "num-override-lvltext.docx")
{
    uno::Reference<container::XIndexAccess> xRules = getProperty< uno::Reference<container::XIndexAccess> >(getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    // This was 1, i.e. the numbering on the second level was "1", not "1.1".
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), comphelper::SequenceAsHashMap(xRules->getByIndex(1))["ParentNumbering"].get<sal_Int16>());

    // The paragraph marker's red font color was inherited by the number portion, this was ff0000.
    CPPUNIT_ASSERT_EQUAL(OUString("00000a"), parseDump("//Special[@nType='POR_NUMBER']/pFont", "color"));
}

DECLARE_OOXMLEXPORT_TEST(testNumOverrideStart, "num-override-start.docx")
{
    uno::Reference<container::XIndexAccess> xRules = getProperty< uno::Reference<container::XIndexAccess> >(getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    // This was 1, i.e. the numbering on the second level was "1.1", not "1.3".
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3), comphelper::SequenceAsHashMap(xRules->getByIndex(1))["StartWith"].get<sal_Int16>());
}

DECLARE_OOXMLEXPORT_TEST(testTdf88583, "tdf88583.odt")
{
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(getParagraph(1), "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x00cc00), getProperty<sal_Int32>(getParagraph(1), "FillColor"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
