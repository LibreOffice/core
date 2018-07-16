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

DECLARE_OOXMLEXPORT_TEST(testfdo81381, "fdo81381.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:object[1]/o:OLEObject[1]",
                    "DrawAspect", "Icon");
}

DECLARE_OOXMLEXPORT_TEST(testChartDupe, "chart-dupe.docx")
{
    // Single chart was exported back as two charts.
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent,
                                                                                    uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(
        xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    // This was 2, on second import we got a duplicated chart copy.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xEmbeddedObjects->getCount());

    xmlDocPtr pXmlDocCT = parseExport("[Content_Types].xml");

    if (!pXmlDocCT)
        return; // initial import

    assertXPath(pXmlDocCT,
                "/ContentType:Types/ContentType:Override[@PartName='/word/charts/chart1.xml']",
                "ContentType", "application/vnd.openxmlformats-officedocument.drawingml.chart+xml");
    assertXPath(pXmlDocCT,
                "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/"
                "Microsoft_Excel_Worksheet1.xlsx']",
                "ContentType", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/charts/_rels/chart1.xml.rels");
    assertXPath(pXmlDocRels,
                "/rels:Relationships/rels:Relationship[@Target='../embeddings/"
                "Microsoft_Excel_Worksheet1.xlsx']",
                "Type",
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocChart1 = parseExport("word/charts/chart1.xml");
    assertXPath(pXmlDocChart1, "/c:chartSpace/c:externalData", "id", "rId1");
}

DECLARE_OOXMLEXPORT_TEST(testFdo51550, "fdo51550.odt")
{
    // The problem was that we lacked the fallback to export the replacement
    // graphic for OLE objects.  But we can actually export the OLE itself now,
    // so check that instead.
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent,
                                                                                    uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(
        xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xEmbeddedObjects->getCount());

    xmlDocPtr pXmlDocCT = parseExport("[Content_Types].xml");

    if (!pXmlDocCT)
        return; // initial import

    assertXPath(
        pXmlDocCT,
        "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.xlsx']",
        "ContentType", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsx']",
        "Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p/w:r/w:object/o:OLEObject", "ProgID",
                "Excel.Sheet.12");
}

DECLARE_OOXMLEXPORT_TEST(testEmbeddedXlsx, "embedded-xlsx.docx")
{
    // check there are two objects and they are FrameShapes
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("FrameShape"), getShape(1)->getShapeType());
    CPPUNIT_ASSERT_EQUAL(OUString("FrameShape"), getShape(2)->getShapeType());

    // check the objects are present in the exported document.xml
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/w:object", 2);

    // finally check the embedded files are present in the zipped document
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    uno::Sequence<OUString> names = xNameAccess->getElementNames();
    int nSheetFiles = 0;
    int nImageFiles = 0;
    for (int i = 0; i < names.getLength(); i++)
    {
        if (names[i].startsWith("word/embeddings/oleObject"))
            nSheetFiles++;
        if (names[i].startsWith("word/media/image"))
            nImageFiles++;
    }
    CPPUNIT_ASSERT_EQUAL(2, nSheetFiles);
    CPPUNIT_ASSERT_EQUAL(2, nImageFiles);
}

DECLARE_OOXMLEXPORT_TEST(testOleObject, "test_ole_object.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:object/v:shape/v:imagedata", "o:title",
                "");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:object/o:OLEObject", "DrawAspect",
                "Content");
    // TODO: ProgID="Package" - what is this? Zip with 10k extra header?

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.bin']",
        "Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject");
    // check the media type too
    xmlDocPtr pXmlDocCT = parseExport("[Content_Types].xml");
    assertXPath(
        pXmlDocCT,
        "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.bin']",
        "ContentType", "application/vnd.openxmlformats-officedocument.oleObject");
}

DECLARE_OOXMLEXPORT_TEST(testFileWithInvalidImageLink, "FileWithInvalidImageLink.docx")
{
    /* In case if the original file has an image whose link is
    invalid, then the RT file used to result in corruption
    since the exported image would be an empty image.
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r[2]/w:drawing[1]/wp:inline[1]/a:graphic[1]/"
                "a:graphicData[1]/pic:pic[1]/pic:blipFill[1]/a:blip[1]",
                "embed", "");
}

DECLARE_OOXMLEXPORT_TEST(testContentTypeDOCX, "fdo80410.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc) // only test the export, not initial import
        return;

    assertXPath(
        pXmlDoc,
        "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.docx']",
        "ContentType", "application/vnd.openxmlformats-officedocument.wordprocessingml.document");
    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.docx']",
        "Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[6]/w:r/w:object/o:OLEObject", "ProgID",
                "Word.Document.12");
}

DECLARE_OOXMLEXPORT_TEST(testEmbeddedExcelChart, "EmbeddedExcelChart.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc) // only test the export, not initial import
        return;

    assertXPath(
        pXmlDoc,
        "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.xls']",
        "ContentType", "application/vnd.ms-excel");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xls']",
        "Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject");

    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p/w:r/w:object/o:OLEObject", "ProgID",
                "Excel.Chart.8");
}

DECLARE_OOXMLEXPORT_TEST(testContentTypeOLE, "fdo77759.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/"
                "vnd.openxmlformats-officedocument.spreadsheetml.sheet']",
                "PartName", "/word/embeddings/oleObject1.xlsx");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsx']",
        "Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject", "ProgID",
                "Excel.Sheet.12");
}

DECLARE_OOXMLEXPORT_TEST(testfdo79968_sldx, "fdo79968.docx")
{
    // This UT for DOCX embedded with powerpoint slide
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/"
                "vnd.openxmlformats-officedocument.presentationml.slide']",
                "PartName", "/word/embeddings/oleObject1.sldx");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.sldx']",
        "Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject", "ProgID",
                "PowerPoint.Slide.12");
}

DECLARE_OOXMLEXPORT_TEST(testfdo79969_xlsb, "fdo79969_xlsb.docx")
{
    // This UT for DOCX embedded with binary excel work sheet.
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/"
                "vnd.ms-excel.sheet.binary.macroEnabled.12']",
                "PartName", "/word/embeddings/oleObject1.xlsb");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsb']",
        "Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject", "ProgID",
                "Excel.SheetBinaryMacroEnabled.12");
}

DECLARE_OOXMLEXPORT_TEST(testfdo79969_xlsm, "fdo79969_xlsm.docx")
{
    // This UT for DOCX embedded with excel work sheet.
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/"
                "vnd.ms-excel.sheet.macroEnabled.12']",
                "PartName", "/word/embeddings/oleObject1.xlsm");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsm']",
        "Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject", "ProgID",
                "Excel.SheetMacroEnabled.12");
}

DECLARE_OOXMLEXPORT_TEST(testfdo80522, "fdo80522.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/"
                "vnd.ms-word.document.macroEnabled.12']",
                "PartName", "/word/embeddings/oleObject1.docm");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.docm']",
        "Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject", "ProgID",
                "Word.DocumentMacroEnabled.12");
}

DECLARE_OOXMLEXPORT_TEST(testfdo80523_pptm, "fdo80523_pptm.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/"
                "vnd.ms-powerpoint.presentation.macroEnabled.12']",
                "PartName", "/word/embeddings/oleObject1.pptm");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.pptm']",
        "Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject", "ProgID",
                "PowerPoint.ShowMacroEnabled.12");
}

DECLARE_OOXMLEXPORT_TEST(testfdo80523_sldm, "fdo80523_sldm.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/"
                "vnd.ms-powerpoint.slide.macroEnabled.12']",
                "PartName", "/word/embeddings/oleObject1.sldm");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.sldm']",
        "Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject", "ProgID",
                "PowerPoint.SlideMacroEnabled.12");
}

DECLARE_OOXMLEXPORT_TEST(testfdo80898, "fdo80898.docx")
{
    // This UT for DOCX embedded with binary excel work sheet.
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/msword']",
                "PartName", "/word/embeddings/oleObject1.doc");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.doc']",
        "Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject", "ProgID",
                "Word.Document.8");
}

DECLARE_OOXMLEXPORT_TEST(testFdo61343, "fdo61343.docx")
{
    // The problem was that there were a groupshape in the doc, followed by an
    // OLE object, and this lead to a crash.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testChartProp, "chart-prop.docx")
{
    // The problem was that chart was not getting parsed in writer module.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage(xDrawPageSupplier->getDrawPage(),
                                                      uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());

    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(15240), getProperty<sal_Int32>(xPropertySet, "Width"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8890), getProperty<sal_Int32>(xPropertySet, "Height"));
}

DECLARE_OOXMLEXPORT_TEST(testChartSize, "chart-size.docx")
{
    // When chart was in a TextFrame, its size was too large.
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent,
                                                                                    uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(
        xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    // This was 10954.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6008),
                         getProperty<sal_Int32>(xEmbeddedObjects->getByIndex(0), "Width"));

    // Layout modified the document when it had this chart.
    uno::Reference<util::XModifiable> xModifiable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, bool(xModifiable->isModified()));
}

DECLARE_OOXMLEXPORT_TEST(mathtype, "mathtype.docx")
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent,
                                                                                    uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(
        xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    // This failed as the Model property was empty.
    auto xModel
        = getProperty<uno::Reference<lang::XServiceInfo>>(xEmbeddedObjects->getByIndex(0), "Model");
    CPPUNIT_ASSERT(xModel->supportsService("com.sun.star.formula.FormulaProperties"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
