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
    void testIndexingExport();

    CPPUNIT_TEST_SUITE(IndexingExportTest);
    CPPUNIT_TEST(testIndexingExport);
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

void IndexingExportTest::testIndexingExport()
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

CPPUNIT_TEST_SUITE_REGISTRATION(IndexingExportTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
