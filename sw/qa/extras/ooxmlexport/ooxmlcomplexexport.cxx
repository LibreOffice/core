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
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <config_features.h>
#include <string>
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }

protected:
};

DECLARE_OOXMLEXPORT_TEST(testFileOpenInputOutputError, "floatingtbl_with_formula.docx")
{
    // Docx containing Floating table with formula was giving "General input/output error" while opening in LibreOffice
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pStyle", "val", "Normal");

    // let's also assert that the formula was exported properly
    assertXPathContent(
        pXmlDoc, "//wps:txbx/w:txbxContent/w:tbl/w:tr/w:tc[2]/w:p/m:oMath/m:sSubSup/m:e/m:r/m:t",
        u"\u03C3");
}

//points
DECLARE_OOXMLEXPORT_TEST(testBezier, "bezier.odt")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    // Check that no shape got lost: a bezier, a line and a text shape.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xDraws->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testFDO71834, "fdo71834.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[2]/w:tc[1]/w:tcPr[1]/w:tcW[1]", "type",
                "dxa");
}

DECLARE_OOXMLEXPORT_TEST(testTableRowDataDisplayedTwice, "table-row-data-displayed-twice.docx")
{
    // fdo#73534: There was a problem for some documents during export.Invalid sectPr getting added
    // because of wrong condition in code.
    // This was the reason for increasing number of pages after RT
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testTdf90697_complexBreaksHeaders, "tdf90697_complexBreaksHeaders.docx")
{
    // This is a complex document using many types of section breaks and re-defined headers.
    // Paragraphs 44-47 were in two columns
    uno::Reference<beans::XPropertySet> xTextSection
        = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(45), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    // after that, the section break should switch things back to one column.
    xTextSection
        = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(50), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());
}

DECLARE_OOXMLEXPORT_TEST(testChartInFooter, "chart-in-footer.docx")
{
    // fdo#73872: document contains chart in footer.
    // The problem was that  footer1.xml.rels files for footer1.xml
    // files were missing from docx file after roundtrip.
    xmlDocPtr pXmlDoc = parseExport("word/_rels/footer1.xml.rels");
    if (!pXmlDoc)
        return;

    // Check footer1.xml.rels contains in doc after roundtrip.
    // Check Id = rId1 in footer1.xml.rels
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship", "Id", "rId1");
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Type",
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart");

    xmlDocPtr pXmlDocCT = parseExport("[Content_Types].xml");
    assertXPath(pXmlDocCT,
                "/ContentType:Types/ContentType:Override[@PartName='/word/charts/chart1.xml']",
                "ContentType", "application/vnd.openxmlformats-officedocument.drawingml.chart+xml");

    // check the content too
    xmlDocPtr pXmlDocFooter1 = parseExport("word/footer1.xml");
    assertXPath(pXmlDocFooter1, "/w:ftr/w:p[1]/w:r/w:drawing/wp:inline/a:graphic/a:graphicData",
                "uri", "http://schemas.openxmlformats.org/drawingml/2006/chart");
    assertXPath(pXmlDocFooter1,
                "/w:ftr/w:p[1]/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/c:chart", "id",
                "rId1");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    if (xDrawPageSupplier.is())
    {
        // If xDrawPage->getCount()==1, then document contains one shape.
        uno::Reference<container::XIndexAccess> xDrawPage(xDrawPageSupplier->getDrawPage(),
                                                          uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount()); // One shape in the doc
    }
}

//smartart
DECLARE_OOXMLEXPORT_TEST(testFdo74792, "fdo74792.docx")
{
    /*
    * fdo#74792 : The images associated with smart-art data[i].xml
    * were not preserved on exporting to DOCX format
    * Added support to grabbag the rels, with associated images.
    */
    xmlDocPtr pXmlDoc = parseExport("word/diagrams/_rels/data1.xml.rels");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship", 4);
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());

    //check that images are also saved
    OUString const sImageFile(
        "word/media/OOXDiagramDataRels1_0.jpeg"); //added anchor id to form a unique name
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName(sImageFile),
                                                  uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInputStream.is());
}

//smartart
DECLARE_OOXMLEXPORT_TEST(testFdo77718, "fdo77718.docx")
{
    //in case of multiple smart arts the names for images were getting
    //repeated and thereby causing a data loss as the binary stream was
    //getting over written. This test case ensures that unique names are
    //given for images in different smart arts.
    xmlDocPtr pXmlDataRels1 = parseExport("word/diagrams/_rels/data1.xml.rels");
    if (!pXmlDataRels1)
        return;

    xmlDocPtr pXmlDataRels2 = parseExport("word/diagrams/_rels/data2.xml.rels");
    if (!pXmlDataRels2)
        return;

    //ensure that the rels file is present.
    assertXPath(pXmlDataRels1, "/rels:Relationships/rels:Relationship", 4);
    assertXPath(pXmlDataRels2, "/rels:Relationships/rels:Relationship", 4);

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());

    //check that images are also saved
    OUString const sImageFile1(
        "word/media/OOXDiagramDataRels1_0.jpeg"); //added anchor id to form a unique name
    uno::Reference<io::XInputStream> xInputStream1(xNameAccess->getByName(sImageFile1),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInputStream1.is());

    //check that images are saved for other smart-arts as well.
    OUString const sImageFile2(
        "word/media/OOXDiagramDataRels2_0.jpeg"); //added anchor id to form a unique name
    uno::Reference<io::XInputStream> xInputStream2(xNameAccess->getByName(sImageFile2),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInputStream2.is());
}

DECLARE_OOXMLEXPORT_TEST(testTableCurruption, "tableCurrupt.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/header2.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/w:hdr/w:tbl[1]/w:tr[1]/w:tc[1]", 1);
}

DECLARE_OOXMLEXPORT_TEST(testFDO76312, "FDO76312.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]");
}

DECLARE_OOXMLEXPORT_TEST(testOLEObjectinHeader, "2129393649.docx")
{
    // fdo#76015 : Document contains oleobject in header xml.
    // Problem was relationship entry for oleobject from header was
    // exported into document.xml.rels file because of this rels file
    // for headers were missing from document/word/rels.
    xmlDocPtr pXmlDoc = parseExport("word/_rels/header1.xml.rels");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[1]", "Id", "rId1");

    xmlDocPtr pXmlDocCT = parseExport("[Content_Types].xml");

    // check the media type too
    assertXPath(
        pXmlDocCT,
        "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.bin']",
        "ContentType", "application/vnd.openxmlformats-officedocument.oleObject");

    // check the content too
    xmlDocPtr pXmlDocHeader1 = parseExport("word/header1.xml");
    assertXPath(pXmlDocHeader1, "/w:hdr/w:tbl/w:tr[1]/w:tc[2]/w:p[1]/w:r/w:object/o:OLEObject",
                "ProgID", "Word.Picture.8");
    xmlDocPtr pXmlDocHeader2 = parseExport("word/header2.xml");
    assertXPath(pXmlDocHeader2, "/w:hdr/w:tbl/w:tr[1]/w:tc[2]/w:p[1]/w:r/w:object/o:OLEObject",
                "ProgID", "Word.Picture.8");
}

DECLARE_OOXMLEXPORT_TEST(testContentTypeXLSM, "fdo76098.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/"
                "Microsoft_Excel_Macro-Enabled_Worksheet1.xlsm']",
                "ContentType", "application/vnd.ms-excel.sheet.macroEnabled.12");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/charts/_rels/chart1.xml.rels");
    assertXPath(pXmlDocRels,
                "/rels:Relationships/rels:Relationship[@Target='../embeddings/"
                "Microsoft_Excel_Macro-Enabled_Worksheet1.xlsm']",
                "Type",
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocChart1 = parseExport("word/charts/chart1.xml");
    assertXPath(pXmlDocChart1, "/c:chartSpace/c:externalData", "id", "rId1");
}

DECLARE_OOXMLEXPORT_TEST(testFDO78292, "FDO78292.docx")
{
    //text node is a leaf node, it should not have any children
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[14]/w:sdt[3]/w:sdtPr[1]/w:text/w14:checked", 0);
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

DECLARE_OOXMLEXPORT_TEST(testTdf102466, "tdf102466.docx")
{
    // the problem was: file is truncated: the first page is missing.
    // More precisely, the table in the first page was clipped.
    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        sal_Int32 nFlyPrtHeight
            = getXPath(pXmlDoc, "(/root/page[1]//fly)[1]/infos/prtBounds", "height").toInt32();
        sal_Int32 nTableHeight
            = getXPath(pXmlDoc, "(/root/page[1]//fly)[1]/tab/infos/bounds", "height").toInt32();
        CPPUNIT_ASSERT_MESSAGE("The table is clipped in a fly frame.",
                               nFlyPrtHeight >= nTableHeight);
    }

    // check how much pages we have
    CPPUNIT_ASSERT_EQUAL(10, getPages());

    // check content of the first page
    {
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xDrawPageSupplier->getDrawPage(),
                                                             uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

        // no border
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xFrame, "LineWidth"));
    }

    // Make sure we have 19 tables created
    {
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(19), xTables->getCount());

        // check the text inside first cell of the first table
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);

        const OUString aMustHaveText = "Requerimientos del  Cliente";
        const OUString aActualText = xCell->getString();

        CPPUNIT_ASSERT(aActualText.indexOf(aMustHaveText) > 0);
    }
}

//}

DECLARE_OOXMLEXPORT_TEST(testTdf92724_continuousBreaksComplex,
                         "tdf92724_continuousBreaksComplex.docx")
{
    //There are 2 page breaks, so there should be 3 pages.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testTdf90697_continuousBreaksComplex2,
                         "tdf92724_continuousBreaksComplex2.docx")
{
    // Continuous section breaks with new headers/footers should not immediately switch to a new page style.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();

    sal_Int16 nPages = xCursor->getPage();
    while (nPages > 0)
    {
        OUString sPageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
        uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
            getStyles("PageStyles")->getByName(sPageStyleName), "HeaderText");
        // Specific case to avoid.  Testing separately (even though redundant).
        // The first header (defined on page 3) ONLY is shown IF the section happens to start on a new page (which it doesn't in this document).
        CPPUNIT_ASSERT(xHeaderText->getString()
                       != "Third section - First page header. No follow defined");
        // Same test stated differently: Pages 4 and 5 OUGHT to use "Second section header", but currently don't.  Page 6 does.
        if (nPages <= 3)
            CPPUNIT_ASSERT_EQUAL(OUString("First section header"), xHeaderText->getString());
        else
            CPPUNIT_ASSERT(xHeaderText->getString() == "First section header"
                           || xHeaderText->getString() == "Second section header");

        xCursor->jumpToPage(--nPages);
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf81345_045Original, "tdf81345.docx")
{
    //Header wasn't replaced  and columns were missing because no new style was created.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);

    xCursor->jumpToPage(2);
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    CPPUNIT_ASSERT(pageStyleName != "Standard");

    // tdf89297 Styles were being added before their base/parent/inherited-from style existed, and so were using default settings.
    uno::Reference<container::XNameAccess> xParaStyles(getStyles("ParagraphStyles"));
    uno::Reference<beans::XPropertySet> xStyle(xParaStyles->getByName("Pull quote"),
                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6736947), getProperty<sal_Int32>(xStyle, "CharColor"));
}

DECLARE_OOXMLEXPORT_TEST(testFDO76248, "FDO76248.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // In two cases the a:graphicData elements had no children, which is invalid.
    assertXPath(pXmlDoc, "//a:graphicData[not(*)]", 0);
}

DECLARE_OOXMLEXPORT_TEST(testfdo79008, "fdo79008.docx")
{
    /* File crashing while saving in LO.
    * Check if document.xml file is created after fix
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
}

DECLARE_OOXMLEXPORT_TEST(testOldComplexMergeTableInTable, "ooo96040-2.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;
}

DECLARE_OOXMLEXPORT_TEST(testfdo78420, "fdo78420.docx")
{
    xmlDocPtr pXmlHeader = parseExport("word/header2.xml");

    if (!pXmlHeader)
        return;

    xmlDocPtr pXmlHeaderRels = parseExport("word/_rels/header2.xml.rels");
    if (!pXmlHeaderRels)
        return;

    assertXPath(pXmlHeaderRels, "/rels:Relationships/rels:Relationship[1]", "Id", "rId1");
}

//smartart
DECLARE_OOXMLEXPORT_TEST(testfdo79822, "fdo79822.docx")
{
    /* File getting crash while saving in LO.
    * The Docx contain smartart and the file was created in ms word 2007
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
}

DECLARE_OOXMLEXPORT_TEST(testSmartArtAnchoredInline, "fdo73227.docx")
{
    /* Given file contains 3 DrawingML objects as 1Picture,1SmartArt and 1Shape.
    * Check for SmartArt.
    *  SmartArt should get written as "Floating Object" i.e. inside <wp:anchor> tag.
    *  Also check for value of attribute "id" of <wp:docPr>. It should be unique for
    *  all 3 DrawingML objects in a document.
    */

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing[1]/wp:anchor/wp:docPr", "id", "1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing[1]/wp:anchor/wp:docPr", "name",
                "Diagram1");

    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp:docPr",
        "id", "2");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp:docPr",
        "name", "10-Point Star 3");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing[2]/wp:anchor/wp:docPr", "id", "3");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing[2]/wp:anchor/wp:docPr", "name",
                "Picture 1");
}

DECLARE_OOXMLEXPORT_TEST(testOuterShdw, "testOuterShdw.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "//mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/"
                "a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:effectLst[1]/a:outerShdw[1]",
                "dist", "57811035");
}

DECLARE_OOXMLEXPORT_TEST(testExtentValue, "fdo74605.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    sal_Int32 nX = getXPath(pXmlDoc,
                            "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/"
                            "w:drawing[1]/wp:anchor[1]/wp:extent",
                            "cx")
                       .toInt32();
    // This was negative.
    CPPUNIT_ASSERT(nX >= 0);
}

// part of tdf#93676, word gives the frame in the exported .docx a huge height,
// because its exported with 255% height percentage from a 255 HeightPercent
// settings, but 255 is a special flag that the value is synced to the
// other dimension.
DECLARE_OOXMLEXPORT_TEST(testSyncedRelativePercent, "tdf93676-1.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // check no explicit pctHeight has been exported, all we care
    // about at this point is that it's not 255000
    assertXPath(pXmlDoc, "//wp14:pctHeight", 0);
}

DECLARE_OOXMLEXPORT_TEST(fdo76591, "fdo76591.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document[1]/w:body[1]/w:p[1]/w:r[3]/mc:AlternateContent[1]/mc:Choice[1]/"
                "w:drawing[1]/wp:anchor[1]",
                "relativeHeight", "3");
}

DECLARE_OOXMLEXPORT_TEST(test77219, "test77219.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document[1]/w:body[1]/w:p[6]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/"
                "w:drawing[1]/wp:anchor[1]",
                "behindDoc", "1");
}

DECLARE_OOXMLEXPORT_TEST(fdo78474, "fdo78474.docx")
{
    xmlDocPtr pXmlDoc1 = parseExport("word/document.xml");
    if (!pXmlDoc1)
        return;
    //docx file after RT is getting corrupted.
    assertXPath(pXmlDoc1,
                "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/"
                "w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:txbx[1]/"
                "w:txbxContent[1]/w:p[1]/w:r[1]/w:drawing[1]/wp:inline[1]/a:graphic[1]/"
                "a:graphicData[1]/pic:pic[1]/pic:blipFill[1]/a:blip[1]",
                "embed", "rId2");

    xmlDocPtr pXmlDoc2 = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc2)
        return;
    assertXPath(pXmlDoc2, "/rels:Relationships/rels:Relationship[2]", "Id", "rId2");
}

DECLARE_OOXMLEXPORT_TEST(testWordArtWithinDraingtool, "testWordArtWithinDraingtool.docx")
{
    /*   * Within a file, there is a 2007 wordArt enclosed in a drawing tool
    * LO was exporting it as below:
    * Sample XML as in Original file:
    * <p> <r> <ac> <drawing> <txbx> <txbxContent> ..  <pict> </pict> </txbxContent></txbx> </drawing> </ac> </r> </p>
    *  After RT :
    * <p> <r> <ac> <drawing> <txbx> <txbxContent> ..  <drawing> <txbx> <txbxContent> ..  </txbxContent></txbx> </drawing> .. </txbxContent></txbx> </drawing> </ac> </r> </p>
    *  Expected : as there is nesting of a 2007 Word Art within a drawing tool, then can be separated in two different runs.
    * */

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent",
                1);
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/"
                "v:textbox/w:txbxContent/w:p/w:r/w:pict/v:shape",
                1);
    // Make sure that the shape inside a shape is exported as VML-only, no embedded mc:AlternateContent before w:pict.
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:pict",
                1);
}

DECLARE_OOXMLEXPORT_TEST(testTextboxRoundedCorners, "textbox-rounded-corners.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    comphelper::SequenceAsHashMap aCustomShapeGeometry = comphelper::SequenceAsHashMap(
        getProperty<uno::Sequence<beans::PropertyValue>>(xShape, "CustomShapeGeometry"));

    // Test that the shape is a rounded rectangle.
    CPPUNIT_ASSERT_EQUAL(OUString("ooxml-roundRect"), aCustomShapeGeometry["Type"].get<OUString>());

    // The shape text should start with a table, with "a" in its A1 cell.
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1, xText), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xCell->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTDF91122, "tdf91122.docx")
{
    /*
    * OLE object shape: default vertical position is top in MSO, not bottom
    */
    { // Check first shape
        uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
        uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xShapeProperties,
                                                                   uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("FrameShape"), xShapeDescriptor->getShapeType());
        sal_Int16 nValue;
        xShapeProperties->getPropertyValue("VertOrient") >>= nValue;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong vertical orientation", text::VertOrientation::TOP,
                                     nValue);
    }

    { // Check second shape
        uno::Reference<beans::XPropertySet> xShapeProperties(getShape(2), uno::UNO_QUERY);
        uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xShapeProperties,
                                                                   uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("FrameShape"), xShapeDescriptor->getShapeType());
        sal_Int16 nValue;
        xShapeProperties->getPropertyValue("VertOrient") >>= nValue;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong vertical orientation", text::VertOrientation::TOP,
                                     nValue);
    }
}

DECLARE_OOXMLEXPORT_TEST(testN780563, "n780563.docx")
{
    /*
    * Make sure we have the table in the fly frame created
    */
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testN830205, "n830205.docx")
{
    // Previously import just crashed (due to infinite recursion).
    getParagraph(1, "XXX");
}

DECLARE_OOXMLEXPORT_TEST(testTdf37153, "tdf37153_considerWrapOnObjPos.docx")
{
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH,
                         getProperty<text::WrapTextMode>(getShape(1), "Surround"));

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::BOTTOM,
                         getProperty<sal_Int16>(xTable->getCellByName("A1"), "VertOrient"));

    //For MSO compatibility, the textbox should be at the top of the cell, not at the bottom - despite VertOrientation::BOTTOM
    xmlDocPtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFlyTop
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt/anchored/fly/infos/bounds", "top")
              .toInt32();
    CPPUNIT_ASSERT_MESSAGE("FlyTop should be 2865, not 5649", nFlyTop < sal_Int32(3000));
    sal_Int32 nTextTop
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[2]/txt[1]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_MESSAGE("TextTop should be 3856", nTextTop > 3000);
}

DECLARE_OOXMLEXPORT_TEST(tdf112169, "tdf112169.odt")
{
    // LO crashed while export because of character background color handling
}

DECLARE_OOXMLEXPORT_TEST(testMissingPath, "missing-path.docx")
{
    comphelper::SequenceAsHashMap aCustomShapeGeometry(
        getProperty<beans::PropertyValues>(getShape(1), "CustomShapeGeometry"));
    comphelper::SequenceAsHashMap aPath(aCustomShapeGeometry["Path"].get<beans::PropertyValues>());
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates
        = aPath["Coordinates"].get<uno::Sequence<drawing::EnhancedCustomShapeParameterPair>>();
    // This was 0, the coordinate list was empty.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(19), aCoordinates.getLength());
}

DECLARE_OOXMLEXPORT_TEST(testLOCrash, "file_crash.docx")
{
    //The problem was libreoffice crash while opening the file.
    getParagraph(1, "Contents");
}

DECLARE_OOXMLEXPORT_TEST(testStrict, "strict.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    // This was only 127, pt suffix was ignored, so this got parsed as twips instead of points.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(72 * 20)),
                         getProperty<sal_Int32>(xPageStyle, "BottomMargin"));
    // This was only 1397, same issue
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(792 * 20)),
                         getProperty<sal_Int32>(xPageStyle, "Height"));
    // Text was missing, due to not handling the strict namespaces.
    getParagraph(1, "Hello world!");

    // Header in the document caused a crash on import.
    uno::Reference<text::XText> xHeaderText(xPageStyle->getPropertyValue("HeaderText"),
                                            uno::UNO_QUERY);
    getParagraphOfText(1, xHeaderText, "This is a header.");

    // Picture was missing.
    uno::Reference<lang::XServiceInfo> xServiceInfo(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextGraphicObject"));

    // SmartArt was missing.
    xServiceInfo.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"));

    // Chart was missing.
    xServiceInfo.set(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextEmbeddedObject"));

    // Math was missing.
    xServiceInfo.set(getShape(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextEmbeddedObject"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo78883, "fdo78883.docx")
{
    // fdo#78883 : LO was getting hang while opening document
    // Checking there is a single page after loading a doc in LO.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();

    // Check to make sure the document loaded.  Note that the page number may
    // be 1 or 2 depending on the environment.
    CPPUNIT_ASSERT(xCursor->getPage() > sal_Int16(0));
}

DECLARE_OOXMLEXPORT_TEST(testFdo78939, "fdo78939.docx")
{
    // fdo#78939 : LO hanged while opening issue document

    // Whenever a para-style was applied to a Numbering format level,
    // LO incorrectly also changed the para-style..

    // check that file opens and does not hang while opening and also
    // check that an incorrect numbering style is not applied ...
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(1), "NumberingStyleName"));
}

DECLARE_OOXMLEXPORT_TEST(testBnc891663, "bnc891663.docx")
{
    // The image should be inside a cell, so the text in the following cell should be below it.
    int imageTop
        = parseDump("/root/page/body/tab/row[1]/cell[2]/txt[1]/anchored/fly/infos/bounds", "top")
              .toInt32();
    int imageHeight
        = parseDump("/root/page/body/tab/row[1]/cell[2]/txt[1]/anchored/fly/infos/bounds", "height")
              .toInt32();
    int textNextRowTop
        = parseDump("/root/page/body/tab/row[2]/cell[1]/txt[1]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT(textNextRowTop >= imageTop + imageHeight);
}

DECLARE_OOXMLEXPORT_TEST(testTdf95775, "tdf95775.docx")
{
    // This must not fail in layout
}

DECLARE_OOXMLEXPORT_TEST(testTdf92157, "tdf92157.docx")
{
    // A graphic with dimensions 0,0 should not fail on load
}

DECLARE_OOXMLEXPORT_TEST(testTdf114882, "tdf114882.docx")
{
    // fastserializer must not fail assertion because of mismatching elements
}

DECLARE_OOXMLEXPORT_TEST(testTableCrossReference, "table_cross_reference.odt")
{
    // tdf#42346: Cross references to tables were not saved
    // MSO uses simple bookmarks for referencing table caption, so we do the same by export
    if (!mbExported)
        return;

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_number_only"));

    // Check bookmark text ranges
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table0_full"),
                                                    uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table 1: Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table 1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
    }

    // Check reference fields
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());

    sal_uInt16 nIndex = 0;
    while (xFields->hasMoreElements())
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xFields->nextElement(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xServiceInfo, uno::UNO_QUERY);
        switch (nIndex)
        {
                // Full reference to table caption
            case 0:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Table 1: Table caption"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_full"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Page style reference / exported as simple page reference
            case 1:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_full"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to table number
            case 2:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_number_only"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to caption only
            case 3:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_caption_only"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to category and number
            case 4:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Table 1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_label_and_number"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to page of the table
            case 5:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_full"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Above / below reference
            case 6:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("above"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_full"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            default:
                break;
        }
        ++nIndex;
    }

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), nIndex);
}

DECLARE_OOXMLEXPORT_TEST(testTableCrossReferenceCustomFormat,
                         "table_cross_reference_custom_format.odt")
{
    // tdf#42346: Cross references to tables were not saved
    // Check also captions with custom formatting
    if (!mbExported)
        return;

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(16), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table1_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table1_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table1_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table1_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table2_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table2_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table2_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table2_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table3_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table3_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table3_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table3_number_only"));

    // Check bookmark text ranges
    // First table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table0_full"),
                                                    uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1. Table: Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1. Table"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
    }
    // Second table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table1_full"),
                                                    uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("2. TableTable caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table1_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("2. Table"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table1_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table1_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("2"), xRange->getString());
    }
    // Third table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table2_full"),
                                                    uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("3) Table Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table2_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("3) Table"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table2_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table2_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("3"), xRange->getString());
    }
    // Fourth table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table3_full"),
                                                    uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table 4- Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table3_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table 4"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table3_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table3_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("4"), xRange->getString());
    }
}

DECLARE_OOXMLEXPORT_TEST(testObjectCrossReference, "object_cross_reference.odt")
{
    // tdf#42346: Cross references to objects were not saved
    // MSO uses simple bookmarks for referencing table caption, so we do the same by export
    if (!mbExported)
        return;

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(15), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing0_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing1_full"));

    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration0_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration1_caption_only"));

    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text0_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text1_label_and_number"));

    // Check bookmark text ranges
    // Cross references to shapes
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Drawing0_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Drawing 1: A rectangle"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Drawing0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Drawing 1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Drawing0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("A rectangle"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Drawing0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Drawing1_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Drawing 2: a circle"), xRange->getString());
    }

    // Cross references to pictures
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Illustration0_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Illustration 1: A picture"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Illustration0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Illustration 1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Illustration0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("A picture"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Illustration0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Illustration1_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("an other image"), xRange->getString());
    }

    // Cross references to text frames
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Text0_full"),
                                                    uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Text 1: A frame"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Text0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Text 1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Text0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("A frame"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Text0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Text1_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Text 2"), xRange->getString());
    }

    // Check reference fields
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());

    sal_uInt16 nIndex = 0;
    while (xFields->hasMoreElements())
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xFields->nextElement(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xServiceInfo, uno::UNO_QUERY);
        switch (nIndex)
        {
                // Reference to image number
            case 0:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Illustration0_number_only"), sValue);
                break;
            }
            // Full reference to the circle shape
            case 1:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Drawing 2: a circle"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Drawing1_full"), sValue);
                break;
            }
            // Caption only reference to the second picture
            case 2:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("an other image"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Illustration1_caption_only"), sValue);
                break;
            }
            // Category and number reference to second text frame
            case 3:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Text 2"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Text1_label_and_number"), sValue);
                break;
            }
            // Full reference to rectangle shape
            case 4:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Drawing 1: A rectangle"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Drawing0_full"), sValue);
                break;
            }
            // Caption only reference to rectangle shape
            case 5:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("A rectangle"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Drawing0_caption_only"), sValue);
                break;
            }
            // Category and number reference to rectangle shape
            case 6:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Drawing 1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Drawing0_label_and_number"), sValue);
                break;
            }
            // Reference to rectangle shape's number
            case 7:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Drawing0_number_only"), sValue);
                break;
            }
            // Full reference to first text frame
            case 8:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Text 1: A frame"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Text0_full"), sValue);
                break;
            }
            // Caption only reference to first text frame
            case 9:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("A frame"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Text0_caption_only"), sValue);
                break;
            }
            // Category and number reference to first text frame
            case 10:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Text 1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Text0_label_and_number"), sValue);
                break;
            }
            // Number only reference to first text frame
            case 11:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Text0_number_only"), sValue);
                break;
            }
            // Full reference to first picture
            case 12:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Illustration 1: A picture"),
                                     sValue.trim()); // failes on MAC without trim
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Illustration0_full"), sValue);
                break;
            }
            // Reference to first picture' caption
            case 13:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("A picture"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Illustration0_caption_only"), sValue);
                break;
            }
            // Category and number reference to first picture
            case 14:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Illustration 1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Illustration0_label_and_number"), sValue);
                break;
            }
            default:
                break;
        }
        ++nIndex;
    }

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(21), nIndex);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
