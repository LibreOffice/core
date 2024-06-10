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
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing"_ostr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]"_ostr, u"Title"_ustr);
    assertXPath(pXmlDoc, "/indexing/paragraph[1]"_ostr, "node_type"_ostr, u"writer"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]"_ostr, u"Heading 1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]"_ostr, u"Heading 2"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]"_ostr, u"Paragraph 1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[5]"_ostr, u"Paragraph 2"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[6]"_ostr, u"Bullet 1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[7]"_ostr, u"Bullet 2"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[8]"_ostr, u"Bullet 3"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[9]"_ostr, u"Paragraph 3"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[10]"_ostr, u"Paragraph 4"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[11]"_ostr, u"List 1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[12]"_ostr, u"List 2"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[13]"_ostr, u"List 3"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[14]"_ostr, u"Left"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[15]"_ostr, u"Center"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[16]"_ostr, u"Right"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[17]"_ostr,
                       u"Bold Italic Underline Strikeout"_ustr);
    assertXPath(pXmlDoc, "/indexing/paragraph[17]"_ostr, "node_type"_ostr, u"writer"_ustr);
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
                u"Image_NonCaption - Alternative text"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "name"_ostr, u"Image_NonCaption"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "object_type"_ostr, u"graphic"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "alt"_ostr,
                u"Image_InCaption - Alternative text"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "name"_ostr, u"Image_InCaption"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "object_type"_ostr, u"graphic"_ustr);
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
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "name"_ostr, u"Object - Chart"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "alt"_ostr, u"Alt Text"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "object_type"_ostr, u"ole"_ustr);
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
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]"_ostr, u"Drawing : Just a Diamond"_ustr);
    assertXPath(pXmlDoc, "/indexing/paragraph[1]"_ostr, "node_type"_ostr, u"writer"_ustr);

    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "name"_ostr, u"Circle"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "object_type"_ostr, u"shape"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]"_ostr, u"This is a circle"_ustr);
    assertXPath(pXmlDoc, "/indexing/paragraph[2]"_ostr, "node_type"_ostr, u"common"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]"_ostr, u"This is a second paragraph"_ustr);
    assertXPath(pXmlDoc, "/indexing/paragraph[3]"_ostr, "node_type"_ostr, u"common"_ustr);

    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "name"_ostr, u"Diamond"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "object_type"_ostr, u"shape"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]"_ostr, u"This is a diamond"_ustr);
    assertXPath(pXmlDoc, "/indexing/paragraph[4]"_ostr, "node_type"_ostr, u"common"_ustr);

    assertXPath(pXmlDoc, "/indexing/object[3]"_ostr, "name"_ostr, u"Text Frame 1"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[3]"_ostr, "object_type"_ostr, u"shape"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[5]"_ostr, u"This is a TextBox - Para1"_ustr);
    assertXPath(pXmlDoc, "/indexing/paragraph[5]"_ostr, "node_type"_ostr, u"common"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[6]"_ostr, u"Para2"_ustr);
    assertXPath(pXmlDoc, "/indexing/paragraph[6]"_ostr, "node_type"_ostr, u"common"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[7]"_ostr, u"Para3"_ustr);
    assertXPath(pXmlDoc, "/indexing/paragraph[7]"_ostr, "node_type"_ostr, u"common"_ustr);
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

    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "name"_ostr, u"Table1"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "object_type"_ostr, u"table"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "index"_ostr, u"9"_ustr);
    // Search paragraph with parent_index = 9
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][1]"_ostr, u"A"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][2]"_ostr, u"B"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][3]"_ostr, u"1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=9][4]"_ostr, u"2"_ustr);

    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "name"_ostr, u"Table2"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "object_type"_ostr, u"table"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "index"_ostr, u"24"_ustr);
    // Search paragraph with parent_index = 24
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][1]"_ostr, u"A"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][2]"_ostr, u"B"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][3]"_ostr, u"C"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][4]"_ostr, u"1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][5]"_ostr, u"10"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][6]"_ostr, u"100"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][7]"_ostr, u"2"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][8]"_ostr, u"20"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][9]"_ostr, u"200"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][10]"_ostr, u"3"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][11]"_ostr, u"30"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][12]"_ostr, u"300"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][13]"_ostr, u"4"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][14]"_ostr, u"40"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=24][15]"_ostr, u"400"_ustr);

    assertXPath(pXmlDoc, "/indexing/object[3]"_ostr, "name"_ostr, u"WeirdTable"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[3]"_ostr, "object_type"_ostr, u"table"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[3]"_ostr, "index"_ostr, u"72"_ustr);
    // Search paragraph with parent_index = 72
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][1]"_ostr, u"A1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][2]"_ostr, u"B1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][3]"_ostr, u"C1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][4]"_ostr, u"D1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][5]"_ostr, u"A2B2"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][6]"_ostr, u"C2D2"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][7]"_ostr, u"A3B3C3D3"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][8]"_ostr, u"A4-1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][9]"_ostr, u"A4-2"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][10]"_ostr, u"B4-1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][11]"_ostr, u"C4-1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][12]"_ostr, u"D4-1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][13]"_ostr, u"D4-2"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][14]"_ostr, u"B4-2"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][15]"_ostr, u"C4-2"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][16]"_ostr, u"A5B5C5"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[@parent_index=72][17]"_ostr, u"D5"_ustr);
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

    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "name"_ostr, u"Section1"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "object_type"_ostr, u"section"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]"_ostr,
                       u"This is a paragraph in a Section1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]"_ostr, u"Section1 - Paragraph 2"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]"_ostr, u"Section1 - Paragraph 3"_ustr);

    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]"_ostr,
                       u"This is a paragraph outside sections"_ustr);

    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "name"_ostr, u"Section2"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[2]"_ostr, "object_type"_ostr, u"section"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[5]"_ostr, u"Section2 - Paragraph 1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[6]"_ostr, u"Section2 - Paragraph 2"_ustr);

    assertXPathContent(pXmlDoc, "/indexing/paragraph[7]"_ostr,
                       u"This is a paragraph outside sections"_ustr);
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

    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "name"_ostr, u"Gray"_ustr);
    assertXPath(pXmlDoc, "/indexing/object[1]"_ostr, "object_type"_ostr, u"shape"_ustr);

    assertXPath(pXmlDoc, "/indexing/paragraph[1]"_ostr, "object_name"_ostr, u"Gray"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]"_ostr, u"Fontwork Text 1"_ustr);
    assertXPath(pXmlDoc, "/indexing/paragraph[2]"_ostr, "object_name"_ostr, u"Gray"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]"_ostr, u"Fontwork Text 2"_ustr);
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

    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]"_ostr, u"Header Text"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]"_ostr, u"Footer Text"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[3]"_ostr, u"Paragraph 1"_ustr);
    assertXPathContent(pXmlDoc, "/indexing/paragraph[4]"_ostr, u"Paragraph 2"_ustr);
}

} // end of anonymous namespace
CPPUNIT_TEST_SUITE_REGISTRATION(IndexingExportTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
