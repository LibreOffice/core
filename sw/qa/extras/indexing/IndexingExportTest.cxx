/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>

#include <IndexingExport.hxx>

class IndexingExportTest : public SwModelTestBase
{
public:
    IndexingExportTest()
        : SwModelTestBase("/sw/qa/extras/indexing/data/")
    {
    }

    void testIndexingExport_Paragraphs();
    void testIndexingExport_Images();
    void testIndexingExport_OLE();
    void testIndexingExport_Shapes();
    void testIndexingExport_Tables();
    void testIndexingExport_Sections();
    void testIndexingExport_Fontwork();
    void testIndexingExport_Header_Footer();

    CPPUNIT_TEST_SUITE(IndexingExportTest);
    CPPUNIT_TEST(testIndexingExport_Paragraphs);
    CPPUNIT_TEST(testIndexingExport_Images);
    CPPUNIT_TEST(testIndexingExport_OLE);
    CPPUNIT_TEST(testIndexingExport_Shapes);
    CPPUNIT_TEST(testIndexingExport_Tables);
    CPPUNIT_TEST(testIndexingExport_Sections);
    CPPUNIT_TEST(testIndexingExport_Fontwork);
    CPPUNIT_TEST(testIndexingExport_Header_Footer);
    CPPUNIT_TEST_SUITE_END();
};

void IndexingExportTest::testIndexingExport_Paragraphs()
{
    createSwDoc("IndexingExport_VariousParagraphs.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]", "Title");
    assertXPath(pXmlDoc, "/indexing/paragraph[1]", "node_type", "writer");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]", "Heading 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]", "Heading 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]", "Paragraph 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[5]", "Paragraph 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[6]", "Bullet 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[7]", "Bullet 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[8]", "Bullet 3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[9]", "Paragraph 3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[10]", "Paragraph 4");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[11]", "List 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[12]", "List 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[13]", "List 3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[14]", "Left");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[15]", "Center");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[16]", "Right");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[17]", "Bold Italic Underline Strikeout");
    assertXPath(pXmlDoc, "/indexing/paragraph[17]", "node_type", "writer");
}

void IndexingExportTest::testIndexingExport_Images()
{
    createSwDoc("IndexingExport_Images.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");
    assertXPath(pXmlDoc, "/indexing/object[1]", "alt", "Image_NonCaption - Alternative text");
    assertXPath(pXmlDoc, "/indexing/object[1]", "name", "Image_NonCaption");
    assertXPath(pXmlDoc, "/indexing/object[1]", "object_type", "graphic");
    assertXPath(pXmlDoc, "/indexing/object[2]", "alt", "Image_InCaption - Alternative text");
    assertXPath(pXmlDoc, "/indexing/object[2]", "name", "Image_InCaption");
    assertXPath(pXmlDoc, "/indexing/object[2]", "object_type", "graphic");
}

void IndexingExportTest::testIndexingExport_OLE()
{
    createSwDoc("IndexingExport_OLE.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");
    assertXPath(pXmlDoc, "/indexing/object[1]", "name", "Object - Chart");
    assertXPath(pXmlDoc, "/indexing/object[1]", "alt", "Alt Text");
    assertXPath(pXmlDoc, "/indexing/object[1]", "object_type", "ole");
}

void IndexingExportTest::testIndexingExport_Shapes()
{
    createSwDoc("IndexingExport_Shapes.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]", "Drawing : Just a Diamond");
    assertXPath(pXmlDoc, "/indexing/paragraph[1]", "node_type", "writer");

    assertXPath(pXmlDoc, "/indexing/object[1]", "name", "Circle");
    assertXPath(pXmlDoc, "/indexing/object[1]", "object_type", "shape");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]", "This is a circle");
    assertXPath(pXmlDoc, "/indexing/paragraph[2]", "node_type", "common");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]", "This is a second paragraph");
    assertXPath(pXmlDoc, "/indexing/paragraph[3]", "node_type", "common");

    assertXPath(pXmlDoc, "/indexing/object[2]", "name", "Diamond");
    assertXPath(pXmlDoc, "/indexing/object[2]", "object_type", "shape");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]", "This is a diamond");
    assertXPath(pXmlDoc, "/indexing/paragraph[4]", "node_type", "common");

    assertXPath(pXmlDoc, "/indexing/object[3]", "name", "Text Frame 1");
    assertXPath(pXmlDoc, "/indexing/object[3]", "object_type", "shape");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[5]", "This is a TextBox - Para1");
    assertXPath(pXmlDoc, "/indexing/paragraph[5]", "node_type", "common");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[6]", "Para2");
    assertXPath(pXmlDoc, "/indexing/paragraph[6]", "node_type", "common");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[7]", "Para3");
    assertXPath(pXmlDoc, "/indexing/paragraph[7]", "node_type", "common");
}

void IndexingExportTest::testIndexingExport_Tables()
{
    createSwDoc("IndexingExport_Tables.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");

    assertXPath(pXmlDoc, "/indexing/object[1]", "name", "Table1");
    assertXPath(pXmlDoc, "/indexing/object[1]", "object_type", "table");
    assertXPath(pXmlDoc, "/indexing/object[1]", "index", "9");
    // Search paragraph with parent_index = 9
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][1]", "A");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][2]", "B");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][3]", "1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][4]", "2");

    assertXPath(pXmlDoc, "/indexing/object[2]", "name", "Table2");
    assertXPath(pXmlDoc, "/indexing/object[2]", "object_type", "table");
    assertXPath(pXmlDoc, "/indexing/object[2]", "index", "24");
    // Search paragraph with parent_index = 24
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][1]", "A");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][2]", "B");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][3]", "C");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][4]", "1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][5]", "10");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][6]", "100");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][7]", "2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][8]", "20");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][9]", "200");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][10]", "3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][11]", "30");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][12]", "300");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][13]", "4");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][14]", "40");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][15]", "400");

    assertXPath(pXmlDoc, "/indexing/object[3]", "name", "WeirdTable");
    assertXPath(pXmlDoc, "/indexing/object[3]", "object_type", "table");
    assertXPath(pXmlDoc, "/indexing/object[3]", "index", "72");
    // Search paragraph with parent_index = 72
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][1]", "A1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][2]", "B1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][3]", "C1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][4]", "D1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][5]", "A2B2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][6]", "C2D2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][7]", "A3B3C3D3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][8]", "A4-1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][9]", "A4-2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][10]", "B4-1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][11]", "C4-1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][12]", "D4-1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][13]", "D4-2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][14]", "B4-2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][15]", "C4-2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][16]", "A5B5C5");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][17]", "D5");
}

void IndexingExportTest::testIndexingExport_Sections()
{
    createSwDoc("IndexingExport_Sections.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");

    assertXPath(pXmlDoc, "/indexing/object[1]", "name", "Section1");
    assertXPath(pXmlDoc, "/indexing/object[1]", "object_type", "section");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]", "This is a paragraph in a Section1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]", "Section1 - Paragraph 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]", "Section1 - Paragraph 3");

    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]", "This is a paragraph outside sections");

    assertXPath(pXmlDoc, "/indexing/object[2]", "name", "Section2");
    assertXPath(pXmlDoc, "/indexing/object[2]", "object_type", "section");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[5]", "Section2 - Paragraph 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[6]", "Section2 - Paragraph 2");

    assertXPathContent(pXmlDoc, "/indexing/paragraph[7]", "This is a paragraph outside sections");
}

void IndexingExportTest::testIndexingExport_Fontwork()
{
    createSwDoc("IndexingExport_Fontwork.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");

    assertXPath(pXmlDoc, "/indexing/object[1]", "name", "Gray");
    assertXPath(pXmlDoc, "/indexing/object[1]", "object_type", "shape");

    assertXPath(pXmlDoc, "/indexing/paragraph[1]", "object_name", "Gray");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]", "Fontwork Text 1");
    assertXPath(pXmlDoc, "/indexing/paragraph[2]", "object_name", "Gray");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]", "Fontwork Text 2");
}

void IndexingExportTest::testIndexingExport_Header_Footer()
{
    createSwDoc("IndexingExport_Header_Footer.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");

    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]", "Header Text");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]", "Footer Text");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]", "Paragraph 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]", "Paragraph 2");
}

CPPUNIT_TEST_SUITE_REGISTRATION(IndexingExportTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
