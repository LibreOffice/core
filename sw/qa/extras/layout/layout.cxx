/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <comphelper/propertysequence.hxx>
#include <test/mtfxmldump.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/extras/layout/data/";

/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter : public SwModelTestBase
{
public:
    void testTdf116830();
    void testTdf116925();
    void testTdf117028();
    void testTdf106390();
    void testTableExtrusion1();
    void testTableExtrusion2();
    void testTdf116848();
    void testTdf117245();
    void testTdf109077();
    void testTdf109137();
    void testForcepoint76();
    void testTdf118058();
    void testTdf117188();
    void testTdf119875();
    void testTdf116989();
    void testTdf122607();

    CPPUNIT_TEST_SUITE(SwLayoutWriter);
    CPPUNIT_TEST(testTdf116830);
    CPPUNIT_TEST(testTdf116925);
    CPPUNIT_TEST(testTdf117028);
    CPPUNIT_TEST(testTdf106390);
    CPPUNIT_TEST(testTableExtrusion1);
    CPPUNIT_TEST(testTableExtrusion2);
    CPPUNIT_TEST(testTdf116848);
    CPPUNIT_TEST(testTdf117245);
    CPPUNIT_TEST(testTdf109077);
    CPPUNIT_TEST(testTdf109137);
    CPPUNIT_TEST(testForcepoint76);
    CPPUNIT_TEST(testTdf118058);
    CPPUNIT_TEST(testTdf117188);
    CPPUNIT_TEST(testTdf119875);
    CPPUNIT_TEST(testTdf116989);
    CPPUNIT_TEST(testTdf122607);
    CPPUNIT_TEST_SUITE_END();

private:
    SwDoc* createDoc(const char* pName = nullptr);
};

SwDoc* SwLayoutWriter::createDoc(const char* pName)
{
    load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

void SwLayoutWriter::testTdf116830()
{
    SwDoc* pDoc = createDoc("tdf116830.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumper.dumpAndParse(*xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Assert that the yellow rectangle (cell background) is painted after the
    // polypolygon (background shape).
    // Background shape: 1.1.1.2
    // Cell background: 1.1.1.3
    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[2]/push[1]/push[1]/fillcolor[@color='#729fcf']", 1);
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[2]/push[1]/push[1]/polypolygon",
                1);

    // This failed: cell background was painted before the background shape.
    assertXPath(pXmlDoc,
                "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/fillcolor[@color='#ffff00']", 1);
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/rect", 1);
}

void SwLayoutWriter::testTdf116925()
{
    SwDoc* pDoc = createDoc("tdf116925.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumper.dumpAndParse(*xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc,
                       "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/push[3]/textarray/text",
                       "hello");
    // This failed, text color was #000000.
    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/push[3]/textcolor[@color='#ffffff']", 1);
}

void SwLayoutWriter::testTdf117028()
{
    SwDoc* pDoc = createDoc("tdf117028.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumper.dumpAndParse(*xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // The only polypolygon in the rendering result was the white background we
    // want to avoid.
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//polypolygon");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);

    // Make sure the text is still rendered.
    assertXPathContent(pXmlDoc, "//textarray/text", "Hello");
}

void SwLayoutWriter::testTdf106390()
{
    SwDoc* pDoc = createDoc("tdf106390.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumper.dumpAndParse(*xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nBottom = getXPath(pXmlDoc, "//sectrectclipregion", "bottom").toInt32();

    // No end point of line segments shall go below the bottom of the clipping area.
    const OString sXPath = "//polyline/point[@y>" + OString::number(nBottom) + "]";

    assertXPath(pXmlDoc, sXPath, 0);
}

void SwLayoutWriter::testTableExtrusion1()
{
    SwDoc* pDoc = createDoc("table-extrusion1.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumper.dumpAndParse(*xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nRight = getXPath(pXmlDoc, "//sectrectclipregion", "right").toInt32();
    sal_Int32 nLeft = (nRight + getXPath(pXmlDoc, "(//rect)[1]", "right").toInt32()) / 2;

    // Expect table borders in right page margin.
    const OString sXPath = "//polyline/point[@x>" + OString::number(nLeft) + " and @x<"
                           + OString::number(nRight) + "]";

    assertXPath(pXmlDoc, sXPath, 4);
}

void SwLayoutWriter::testTableExtrusion2()
{
    SwDoc* pDoc = createDoc("table-extrusion2.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumper.dumpAndParse(*xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // End point position of the outer table.
    sal_Int32 nX = getXPath(pXmlDoc, "(//polyline[1]/point)[2]", "x").toInt32();

    // Do not allow inner table extrude outer table.
    const OString sXPath = "//polyline/point[@x>" + OString::number(nX) + "]";

    assertXPath(pXmlDoc, sXPath, 0);
}

void SwLayoutWriter::testTdf116848()
{
    SwDoc* pDoc = createDoc("tdf116848.odt");
    // This resulted in a layout loop.
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
}

void SwLayoutWriter::testTdf117245()
{
    createDoc("tdf117245.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // This was 2, TabOverMargin did not use a single line when there was
    // enough space for the text.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak", 1);

    // This was 2, same problem elsewhere due to code duplication.
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/LineBreak", 1);
}

void SwLayoutWriter::testTdf109077()
{
    createDoc("tdf109077.docx");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    sal_Int32 nShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds", "top").toInt32();
    sal_Int32 nTextBoxTop = getXPath(pXmlDoc, "//anchored/fly/infos/bounds", "top").toInt32();
    // This was 281: the top of the shape and its textbox should match, though
    // tolerate differences <= 1px (about 15 twips).
    CPPUNIT_ASSERT_LESS(static_cast<sal_Int32>(15), nTextBoxTop - nShapeTop);
}

void SwLayoutWriter::testTdf109137()
{
    createDoc("tdf109137.docx");
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
    }));
    xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
    loadURL(aTempFile.GetURL(), "tdf109137.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // This was 0, the blue rectangle moved from the 1st to the 2nd page.
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly/notxt",
                /*nNumberOfNodes=*/1);
}

//just care it doesn't crash/assert
void SwLayoutWriter::testForcepoint76() { createDoc("forcepoint76-1.rtf"); }

void SwLayoutWriter::testTdf118058()
{
    SwDoc* pDoc = createDoc("tdf118058.fodt");
    // This resulted in a layout loop.
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
}

void SwLayoutWriter::testTdf117188()
{
    createDoc("tdf117188.docx");
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
    }));
    xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
    loadURL(aTempFile.GetURL(), "tdf117188.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    OUString sWidth = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "width");
    OUString sHeight = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "height");
    // The text box must have zero border distances
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "left", "0");
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "top", "0");
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "width", sWidth);
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "height", sHeight);
}

void SwLayoutWriter::testTdf119875()
{
    createDoc("tdf119875.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFirstTop
        = getXPath(pXmlDoc, "/root/page[2]/body/section[1]/infos/bounds", "top").toInt32();
    sal_Int32 nSecondTop
        = getXPath(pXmlDoc, "/root/page[2]/body/section[2]/infos/bounds", "top").toInt32();
    // The first section had the same top value as the second one, so they
    // overlapped.
    CPPUNIT_ASSERT_LESS(nSecondTop, nFirstTop);
}

void SwLayoutWriter::testTdf116989()
{
    createDoc("tdf116989.docx");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // FIXME: the XPath should be adjusted when the proper floating table would be imported
    const sal_Int32 nTblTop
        = getXPath(pXmlDoc, "/root/page[1]/footer/tab/infos/bounds", "top").toInt32();
    const sal_Int32 nFirstPageParaCount
        = getXPathContent(pXmlDoc, "count(/root/page[1]/body/txt)").toInt32();
    // FIXME: should be exactly 30, when proper floating tables in footers are supported
    CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(30), nFirstPageParaCount);
    for (sal_Int32 i = 1; i <= nFirstPageParaCount; ++i)
    {
        const OString xPath = "/root/page[1]/body/txt[" + OString::number(i) + "]/infos/bounds";
        const sal_Int32 nTxtBottom = getXPath(pXmlDoc, xPath.getStr(), "top").toInt32()
                                     + getXPath(pXmlDoc, xPath.getStr(), "height").toInt32();
        // No body paragraphs should overlap the table in the footer
        CPPUNIT_ASSERT_MESSAGE(OString("testing paragraph #" + OString::number(i)).getStr(),
                               nTxtBottom <= nTblTop);
    }
}

void SwLayoutWriter::testTdf122607()
{
    createDoc("tdf122607.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/txt[1]/anchored/fly/tab/row[2]/cell/txt[7]/anchored/"
                "fly/txt/Text[1]",
                "nHeight", "253");
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/txt[1]/anchored/fly/tab/row[2]/cell/txt[7]/anchored/"
                "fly/txt/Text[1]",
                "nWidth", "428");
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwLayoutWriter);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
