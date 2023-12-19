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

namespace
{
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

    assertXPath(pXmlDoc, "/indexing"_ostr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]"_ostr, "Title");
    assertXPath(pXmlDoc, "/indexing/paragraph[1]"_ostr, "node_type"_ostr, "writer");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]"_ostr, "Heading 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]"_ostr, "Heading 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]"_ostr, "Paragraph 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[5]"_ostr, "Paragraph 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[6]"_ostr, "Bullet 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[7]"_ostr, "Bullet 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[8]"_ostr, "Bullet 3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[9]"_ostr, "Paragraph 3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[10]"_ostr, "Paragraph 4");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[11]"_ostr, "List 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[12]"_ostr, "List 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[13]"_ostr, "List 3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[14]"_ostr, "Left");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[15]"_ostr, "Center");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[16]"_ostr, "Right");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[17]"_ostr, "Bold Italic Underline Strikeout");
    assertXPath(pXmlDoc, "/indexing/paragraph[17]"_ostr, "node_type"_ostr, "writer");
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

    assertXPath(pXmlDoc, "/indexing"_ostr);
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "alt"_ostr,
                "Image_NonCaption - Alternative text");
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "name"_ostr, "Image_NonCaption");
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "object_type"_ostr, "graphic");
    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "alt"_ostr,
                "Image_InCaption - Alternative text");
    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "name"_ostr, "Image_InCaption");
    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "object_type"_ostr, "graphic");
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

    assertXPath(pXmlDoc, "/indexing"_ostr);
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "name"_ostr, "Object - Chart");
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "alt"_ostr, "Alt Text");
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "object_type"_ostr, "ole");
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

    assertXPath(pXmlDoc, "/indexing"_ostr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]"_ostr, "Drawing : Just a Diamond");
    assertXPath(pXmlDoc, "/indexing/paragraph[1]"_ostr, "node_type"_ostr, "writer");

    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "name"_ostr, "Circle");
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "object_type"_ostr, "shape");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]"_ostr, "This is a circle");
    assertXPath(pXmlDoc, "/indexing/paragraph[2]"_ostr, "node_type"_ostr, "common");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]"_ostr, "This is a second paragraph");
    assertXPath(pXmlDoc, "/indexing/paragraph[3]"_ostr, "node_type"_ostr, "common");

    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "name"_ostr, "Diamond");
    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "object_type"_ostr, "shape");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]"_ostr, "This is a diamond");
    assertXPath(pXmlDoc, "/indexing/paragraph[4]"_ostr, "node_type"_ostr, "common");

    assertXPath(pXmlDoc, "/indexing/object[3]"_ostr, "name"_ostr, "Text Frame 1");
    assertXPath(pXmlDoc, "/indexing/object[3]"_ostr, "object_type"_ostr, "shape");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[5]"_ostr, "This is a TextBox - Para1");
    assertXPath(pXmlDoc, "/indexing/paragraph[5]"_ostr, "node_type"_ostr, "common");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[6]"_ostr, "Para2");
    assertXPath(pXmlDoc, "/indexing/paragraph[6]"_ostr, "node_type"_ostr, "common");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[7]"_ostr, "Para3");
    assertXPath(pXmlDoc, "/indexing/paragraph[7]"_ostr, "node_type"_ostr, "common");
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

    assertXPath(pXmlDoc, "/indexing"_ostr);

    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "name"_ostr, "Table1");
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "object_type"_ostr, "table");
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "index"_ostr, "9");
    // Search paragraph with parent_index = 9
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][1]"_ostr, "A");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][2]"_ostr, "B");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][3]"_ostr, "1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][4]"_ostr, "2");

    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "name"_ostr, "Table2");
    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "object_type"_ostr, "table");
    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "index"_ostr, "24");
    // Search paragraph with parent_index = 24
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][1]"_ostr, "A");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][2]"_ostr, "B");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][3]"_ostr, "C");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][4]"_ostr, "1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][5]"_ostr, "10");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][6]"_ostr, "100");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][7]"_ostr, "2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][8]"_ostr, "20");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][9]"_ostr, "200");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][10]"_ostr, "3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][11]"_ostr, "30");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][12]"_ostr, "300");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][13]"_ostr, "4");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][14]"_ostr, "40");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][15]"_ostr, "400");

    assertXPath(pXmlDoc, "/indexing/object[3]"_ostr, "name"_ostr, "WeirdTable");
    assertXPath(pXmlDoc, "/indexing/object[3]"_ostr, "object_type"_ostr, "table");
    assertXPath(pXmlDoc, "/indexing/object[3]"_ostr, "index"_ostr, "72");
    // Search paragraph with parent_index = 72
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][1]"_ostr, "A1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][2]"_ostr, "B1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][3]"_ostr, "C1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][4]"_ostr, "D1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][5]"_ostr, "A2B2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][6]"_ostr, "C2D2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][7]"_ostr, "A3B3C3D3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][8]"_ostr, "A4-1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][9]"_ostr, "A4-2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][10]"_ostr, "B4-1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][11]"_ostr, "C4-1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][12]"_ostr, "D4-1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][13]"_ostr, "D4-2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][14]"_ostr, "B4-2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][15]"_ostr, "C4-2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][16]"_ostr, "A5B5C5");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][17]"_ostr, "D5");
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

    assertXPath(pXmlDoc, "/indexing"_ostr);

    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "name"_ostr, "Section1");
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "object_type"_ostr, "section");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]"_ostr, "This is a paragraph in a Section1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]"_ostr, "Section1 - Paragraph 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]"_ostr, "Section1 - Paragraph 3");

    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]"_ostr,
                       "This is a paragraph outside sections");

    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "name"_ostr, "Section2");
    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "object_type"_ostr, "section");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[5]"_ostr, "Section2 - Paragraph 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[6]"_ostr, "Section2 - Paragraph 2");

    assertXPathContent(pXmlDoc, "/indexing/paragraph[7]"_ostr,
                       "This is a paragraph outside sections");
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

    assertXPath(pXmlDoc, "/indexing"_ostr);

    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "name"_ostr, "Gray");
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "object_type"_ostr, "shape");

    assertXPath(pXmlDoc, "/indexing/paragraph[1]"_ostr, "object_name"_ostr, "Gray");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]"_ostr, "Fontwork Text 1");
    assertXPath(pXmlDoc, "/indexing/paragraph[2]"_ostr, "object_name"_ostr, "Gray");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]"_ostr, "Fontwork Text 2");
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

    assertXPath(pXmlDoc, "/indexing"_ostr);

    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]"_ostr, "Header Text");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]"_ostr, "Footer Text");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]"_ostr, "Paragraph 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]"_ostr, "Paragraph 2");
}

} // end of anonymous namespace
CPPUNIT_TEST_SUITE_REGISTRATION(IndexingExportTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
