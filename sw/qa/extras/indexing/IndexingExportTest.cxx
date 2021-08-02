/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <string_view>
#include <swmodeltestbase.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>

#include <IndexingExport.hxx>

namespace
{
constexpr OUStringLiteral DATA_DIRECTORY = u"sw/qa/extras/indexing/data/";
}

class IndexingExportTest : public SwModelTestBase
{
private:
    SwDoc* createDoc(const char* pName = nullptr);

public:
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

SwDoc* IndexingExportTest::createDoc(const char* pName)
{
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

void IndexingExportTest::testIndexingExport_Paragraphs()
{
    SwDoc* pDoc = createDoc("IndexingExport_VariousParagraphs.odt");
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]", "Title");
    assertXPath(pXmlDoc, "/indexing/paragraph[1]", "type", "1");
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
}

void IndexingExportTest::testIndexingExport_Images()
{
    SwDoc* pDoc = createDoc("IndexingExport_Images.odt");
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");
    assertXPath(pXmlDoc, "/indexing/graphic[1]", "alt", "Image_NonCaption - Alternative text");
    assertXPath(pXmlDoc, "/indexing/graphic[1]", "name", "Image_NonCaption");
    assertXPath(pXmlDoc, "/indexing/graphic[2]", "alt", "Image_InCaption - Alternative text");
    assertXPath(pXmlDoc, "/indexing/graphic[2]", "name", "Image_InCaption");
}

void IndexingExportTest::testIndexingExport_OLE()
{
    SwDoc* pDoc = createDoc("IndexingExport_OLE.odt");
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");
    assertXPath(pXmlDoc, "/indexing/ole[1]", "name", "Object - Chart");
    assertXPath(pXmlDoc, "/indexing/ole[1]", "alt", "Alt Text");
}

void IndexingExportTest::testIndexingExport_Shapes()
{
    SwDoc* pDoc = createDoc("IndexingExport_Shapes.odt");
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");

    assertXPath(pXmlDoc, "/indexing/shape[1]", "name", "Circle");
    assertXPathContent(pXmlDoc, "/indexing/shape[1]/paragraph[1]", "This is a circle");
    assertXPathContent(pXmlDoc, "/indexing/shape[1]/paragraph[2]", "This is a second paragraph");

    assertXPath(pXmlDoc, "/indexing/shape[2]", "name", "Diamond");
    assertXPathContent(pXmlDoc, "/indexing/shape[2]/paragraph[1]", "This is a diamond");

    assertXPath(pXmlDoc, "/indexing/shape[3]", "name", "Text Frame 1");
    assertXPathContent(pXmlDoc, "/indexing/shape[3]/paragraph[1]", "This is a TextBox - Para1");
    assertXPathContent(pXmlDoc, "/indexing/shape[3]/paragraph[2]", "Para2");
    assertXPathContent(pXmlDoc, "/indexing/shape[3]/paragraph[3]", "Para3");
}

void IndexingExportTest::testIndexingExport_Tables()
{
    SwDoc* pDoc = createDoc("IndexingExport_Tables.odt");
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");

    assertXPath(pXmlDoc, "/indexing/table[1]", "name", "Table1");
    assertXPathContent(pXmlDoc, "/indexing/table[1]/paragraph[1]", "A");
    assertXPathContent(pXmlDoc, "/indexing/table[1]/paragraph[2]", "B");
    assertXPathContent(pXmlDoc, "/indexing/table[1]/paragraph[3]", "1");
    assertXPathContent(pXmlDoc, "/indexing/table[1]/paragraph[4]", "2");

    assertXPath(pXmlDoc, "/indexing/table[2]", "name", "Table2");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[1]", "A");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[2]", "B");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[3]", "C");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[4]", "1");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[5]", "10");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[6]", "100");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[7]", "2");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[8]", "20");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[9]", "200");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[10]", "3");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[11]", "30");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[12]", "300");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[13]", "4");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[14]", "40");
    assertXPathContent(pXmlDoc, "/indexing/table[2]/paragraph[15]", "400");

    assertXPath(pXmlDoc, "/indexing/table[3]", "name", "WeirdTable");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[1]", "A1");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[2]", "B1");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[3]", "C1");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[4]", "D1");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[5]", "A2B2");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[6]", "C2D2");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[7]", "A3B3C3D3");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[8]", "A4-1");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[9]", "A4-2");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[10]", "B4-1");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[11]", "C4-1");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[12]", "D4-1");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[13]", "D4-2");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[14]", "");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[15]", "");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[16]", "B4-2");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[17]", "C4-2");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[18]", "");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[19]", "");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[20]", "A5B5C5");
    assertXPathContent(pXmlDoc, "/indexing/table[3]/paragraph[21]", "D5");
}

void IndexingExportTest::testIndexingExport_Sections()
{
    SwDoc* pDoc = createDoc("IndexingExport_Sections.odt");
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");

    assertXPath(pXmlDoc, "/indexing/section[1]", "name", "Section1");
    assertXPathContent(pXmlDoc, "/indexing/section[1]/paragraph[1]",
                       "This is a paragraph in a Section1");
    assertXPathContent(pXmlDoc, "/indexing/section[1]/paragraph[2]", "Section1 - Paragraph 2");
    assertXPathContent(pXmlDoc, "/indexing/section[1]/paragraph[3]", "Section1 - Paragraph 3");

    assertXPath(pXmlDoc, "/indexing/section[2]", "name", "Section2");
    assertXPathContent(pXmlDoc, "/indexing/section[2]/paragraph[1]", "Section2 - Paragraph 1");
    assertXPathContent(pXmlDoc, "/indexing/section[2]/paragraph[2]", "Section2 - Paragraph 2");

    assertXPathContent(pXmlDoc, "/indexing/paragraph[1]", "This is a paragraph outside sections");
    assertXPathContent(pXmlDoc, "/indexing/paragraph[2]", "This is a paragraph outside sections");
}

void IndexingExportTest::testIndexingExport_Fontwork()
{
    SwDoc* pDoc = createDoc("IndexingExport_Fontwork.odt");
    CPPUNIT_ASSERT(pDoc);

    SvMemoryStream aMemoryStream;
    sw::IndexingExport aIndexingExport(aMemoryStream, pDoc);
    aIndexingExport.runExport();
    aMemoryStream.Seek(0);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aMemoryStream);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/indexing");

    assertXPath(pXmlDoc, "/indexing/shape[1]", "name", "Gray");

    assertXPathContent(pXmlDoc, "/indexing/shape[1]/paragraph[1]", "Fontwork Text 1");
    assertXPathContent(pXmlDoc, "/indexing/shape[1]/paragraph[2]", "Fontwork Text 2");
}

void IndexingExportTest::testIndexingExport_Header_Footer()
{
    SwDoc* pDoc = createDoc("IndexingExport_Header_Footer.odt");
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
