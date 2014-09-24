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
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>

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
    {
        xmlXPathObjectPtr pXPath = getXPathNode(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:shd");
        xmlNodeSetPtr pXmlNodes = pXPath->nodesetval;
        CPPUNIT_ASSERT_EQUAL(1, xmlXPathNodeSetGetLength(pXmlNodes));
        xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
        // The attribute existed, so xmlGetProp() returned non-NULL.
        CPPUNIT_ASSERT_EQUAL(static_cast<xmlChar*>(0), xmlGetProp(pXmlNode, BAD_CAST("themeColor")));
    }
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
