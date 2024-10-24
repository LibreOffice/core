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
        : SwModelTestBase(u"/sw/qa/extras/indexing/data/"_ustr)
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

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]", u"Title");
    assertXPath(pXmlDoc, "/indexing/paragraph[1]", "node_type", u"writer");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]", u"Heading 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]", u"Heading 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]", u"Paragraph 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[5]", u"Paragraph 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[6]", u"Bullet 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[7]", u"Bullet 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[8]", u"Bullet 3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[9]", u"Paragraph 3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[10]", u"Paragraph 4");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[11]", u"List 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[12]", u"List 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[13]", u"List 3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[14]", u"Left");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[15]", u"Center");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[16]", u"Right");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[17]", u"Bold Italic Underline Strikeout");
    assertXPath(pXmlDoc, "/indexing/paragraph[17]", "node_type", u"writer");
}

void IndexingExportTest::testIndexingExport_Images()
{
    createSwDoc("IndexingExport_Images.odt");
    SwDoc* pDoc = getSwDoc();

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");
    assertXPath(pXmlDoc, "/indexing/object[1]", "alt", u"Image_NonCaption - Alternative text");
    assertXPath(pXmlDoc, "/indexing/object[1]", "name", u"Image_NonCaption");
    assertXPath(pXmlDoc, "/indexing/object[1]", "object_type", u"graphic");
    assertXPath(pXmlDoc, "/indexing/object[2]", "alt", u"Image_InCaption - Alternative text");
    assertXPath(pXmlDoc, "/indexing/object[2]", "name", u"Image_InCaption");
    assertXPath(pXmlDoc, "/indexing/object[2]", "object_type", u"graphic");
}

void IndexingExportTest::testIndexingExport_OLE()
{
    createSwDoc("IndexingExport_OLE.odt");
    SwDoc* pDoc = getSwDoc();

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");
    assertXPath(pXmlDoc, "/indexing/object[1]", "name", u"Object - Chart");
    assertXPath(pXmlDoc, "/indexing/object[1]", "alt", u"Alt Text");
    assertXPath(pXmlDoc, "/indexing/object[1]", "object_type", u"ole");
}

void IndexingExportTest::testIndexingExport_Shapes()
{
    createSwDoc("IndexingExport_Shapes.odt");
    SwDoc* pDoc = getSwDoc();

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]", u"Drawing : Just a Diamond");
    assertXPath(pXmlDoc, "/indexing/paragraph[1]", "node_type", u"writer");

    assertXPath(pXmlDoc, "/indexing/object[1]", "name", u"Circle");
    assertXPath(pXmlDoc, "/indexing/object[1]", "object_type", u"shape");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]", u"This is a circle");
    assertXPath(pXmlDoc, "/indexing/paragraph[2]", "node_type", u"common");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]", u"This is a second paragraph");
    assertXPath(pXmlDoc, "/indexing/paragraph[3]", "node_type", u"common");

    assertXPath(pXmlDoc, "/indexing/object[2]", "name", u"Diamond");
    assertXPath(pXmlDoc, "/indexing/object[2]", "object_type", u"shape");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]", u"This is a diamond");
    assertXPath(pXmlDoc, "/indexing/paragraph[4]", "node_type", u"common");

    assertXPath(pXmlDoc, "/indexing/object[3]", "name", u"Text Frame 1");
    assertXPath(pXmlDoc, "/indexing/object[3]", "object_type", u"shape");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[5]", u"This is a TextBox - Para1");
    assertXPath(pXmlDoc, "/indexing/paragraph[5]", "node_type", u"common");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[6]", u"Para2");
    assertXPath(pXmlDoc, "/indexing/paragraph[6]", "node_type", u"common");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[7]", u"Para3");
    assertXPath(pXmlDoc, "/indexing/paragraph[7]", "node_type", u"common");
}

void IndexingExportTest::testIndexingExport_Tables()
{
    createSwDoc("IndexingExport_Tables.odt");
    SwDoc* pDoc = getSwDoc();

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");

    assertXPath(pXmlDoc, "/indexing/object[1]", "name", u"Table1");
    assertXPath(pXmlDoc, "/indexing/object[1]", "object_type", u"table");
    assertXPath(pXmlDoc, "/indexing/object[1]", "index", u"9");
    // Search paragraph with parent_index = 9
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][1]", u"A");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][2]", u"B");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][3]", u"1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][4]", u"2");

    assertXPath(pXmlDoc, "/indexing/object[2]", "name", u"Table2");
    assertXPath(pXmlDoc, "/indexing/object[2]", "object_type", u"table");
    assertXPath(pXmlDoc, "/indexing/object[2]", "index", u"24");
    // Search paragraph with parent_index = 24
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][1]", u"A");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][2]", u"B");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][3]", u"C");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][4]", u"1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][5]", u"10");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][6]", u"100");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][7]", u"2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][8]", u"20");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][9]", u"200");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][10]", u"3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][11]", u"30");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][12]", u"300");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][13]", u"4");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][14]", u"40");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][15]", u"400");

    assertXPath(pXmlDoc, "/indexing/object[3]", "name", u"WeirdTable");
    assertXPath(pXmlDoc, "/indexing/object[3]", "object_type", u"table");
    assertXPath(pXmlDoc, "/indexing/object[3]", "index", u"72");
    // Search paragraph with parent_index = 72
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][1]", u"A1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][2]", u"B1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][3]", u"C1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][4]", u"D1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][5]", u"A2B2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][6]", u"C2D2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][7]", u"A3B3C3D3");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][8]", u"A4-1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][9]", u"A4-2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][10]", u"B4-1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][11]", u"C4-1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][12]", u"D4-1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][13]", u"D4-2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][14]", u"B4-2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][15]", u"C4-2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][16]", u"A5B5C5");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][17]", u"D5");
}

void IndexingExportTest::testIndexingExport_Sections()
{
    createSwDoc("IndexingExport_Sections.odt");
    SwDoc* pDoc = getSwDoc();

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");

    assertXPath(pXmlDoc, "/indexing/object[1]", "name", u"Section1");
    assertXPath(pXmlDoc, "/indexing/object[1]", "object_type", u"section");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]", u"This is a paragraph in a Section1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]", u"Section1 - Paragraph 2");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]", u"Section1 - Paragraph 3");

    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]", u"This is a paragraph outside sections");

    assertXPath(pXmlDoc, "/indexing/object[2]", "name", u"Section2");
    assertXPath(pXmlDoc, "/indexing/object[2]", "object_type", u"section");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[5]", u"Section2 - Paragraph 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[6]", u"Section2 - Paragraph 2");

    assertXPathContent(pXmlDoc, "/indexing/paragraph[7]", u"This is a paragraph outside sections");
}

void IndexingExportTest::testIndexingExport_Fontwork()
{
    createSwDoc("IndexingExport_Fontwork.odt");
    SwDoc* pDoc = getSwDoc();

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");

    assertXPath(pXmlDoc, "/indexing/object[1]", "name", u"Gray");
    assertXPath(pXmlDoc, "/indexing/object[1]", "object_type", u"shape");

    assertXPath(pXmlDoc, "/indexing/paragraph[1]", "object_name", u"Gray");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]", u"Fontwork Text 1");
    assertXPath(pXmlDoc, "/indexing/paragraph[2]", "object_name", u"Gray");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]", u"Fontwork Text 2");
}

void IndexingExportTest::testIndexingExport_Header_Footer()
{
    createSwDoc("IndexingExport_Header_Footer.odt");
    SwDoc* pDoc = getSwDoc();

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");

    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]", u"Header Text");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]", u"Footer Text");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]", u"Paragraph 1");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]", u"Paragraph 2");
}

} // end of anonymous namespace
CPPUNIT_TEST_SUITE_REGISTRATION(IndexingExportTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
