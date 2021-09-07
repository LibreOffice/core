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

#include <SearchResultLocator.hxx>

namespace
{
constexpr OUStringLiteral DATA_DIRECTORY = u"sw/qa/extras/indexing/data/";
}

class SearchResultLocatorTest : public SwModelTestBase
{
private:
    SwDoc* createDoc(const char* pName = nullptr);

public:
    void testSearchResultLocator();
    void testSearchResultLocatorUsingXmlPayload();
    void testSearchResultLocatorUsingJsonPayload();
    void testSearchResultLocatorForSdrObjects();

    CPPUNIT_TEST_SUITE(SearchResultLocatorTest);
    CPPUNIT_TEST(testSearchResultLocator);
    CPPUNIT_TEST(testSearchResultLocatorUsingXmlPayload);
    CPPUNIT_TEST(testSearchResultLocatorUsingJsonPayload);
    CPPUNIT_TEST(testSearchResultLocatorForSdrObjects);
    CPPUNIT_TEST_SUITE_END();
};

SwDoc* SearchResultLocatorTest::createDoc(const char* pName)
{
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

void SearchResultLocatorTest::testSearchResultLocator()
{
    if (!IsDefaultDPI())
        return;

    SwDoc* pDoc = createDoc("IndexingExport_VariousParagraphs.odt");
    CPPUNIT_ASSERT(pDoc);

    sw::search::SearchResultLocator aLocator(pDoc);
    std::vector<sw::search::SearchIndexData> aDataVector;
    aDataVector.emplace_back(sw::search::NodeType::WriterNode, 14);

    sw::search::LocationResult aResult = aLocator.find(aDataVector);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aResult.maRectangles.size());

    // skip asserting exact values for macOS and Windows because of
    // inconsistent results
#if !defined(_WIN32) && !defined(MACOSX)
    auto aRectangle = aResult.maRectangles[0];
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1418.0, aRectangle.getMinX(), 1e-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4444.0, aRectangle.getMinY(), 1e-4);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(9638.0, aRectangle.getWidth(), 1e-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(276.0, aRectangle.getHeight(), 1e-4);
#endif
}

void SearchResultLocatorTest::testSearchResultLocatorUsingXmlPayload()
{
    if (!IsDefaultDPI())
        return;

    SwDoc* pDoc = createDoc("IndexingExport_VariousParagraphs.odt");
    CPPUNIT_ASSERT(pDoc);

    sw::search::SearchResultLocator aLocator(pDoc);
    OString payload = "<indexing>"
                      "<paragraph node_type=\"writer\" index=\"14\" />"
                      "</indexing>";

    sw::search::LocationResult aResult = aLocator.findForPayload(payload.getStr());
    CPPUNIT_ASSERT_EQUAL(size_t(1), aResult.maRectangles.size());

    // skip asserting exact values for macOS and Windows because of
    // inconsistent results
#if !defined(_WIN32) && !defined(MACOSX)
    auto aRectangle = aResult.maRectangles[0];
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1418.0, aRectangle.getMinX(), 1e-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4444.0, aRectangle.getMinY(), 1e-4);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(9638.0, aRectangle.getWidth(), 1e-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(276.0, aRectangle.getHeight(), 1e-4);
#endif
}

void SearchResultLocatorTest::testSearchResultLocatorUsingJsonPayload()
{
    if (!IsDefaultDPI())
        return;

    SwDoc* pDoc = createDoc("IndexingExport_VariousParagraphs.odt");
    CPPUNIT_ASSERT(pDoc);

    sw::search::SearchResultLocator aLocator(pDoc);
    OString payload = "["
                      "{ \"node_type\" : \"writer\", \"index\" : 14 }"
                      "]";

    sw::search::LocationResult aResult = aLocator.findForPayload(payload.getStr());
    CPPUNIT_ASSERT_EQUAL(size_t(1), aResult.maRectangles.size());

    // skip asserting exact values for macOS and Windows because of
    // inconsistent results
#if !defined(_WIN32) && !defined(MACOSX)
    auto aRectangle = aResult.maRectangles[0];
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1418.0, aRectangle.getMinX(), 1e-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4444.0, aRectangle.getMinY(), 1e-4);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(9638.0, aRectangle.getWidth(), 1e-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(276.0, aRectangle.getHeight(), 1e-4);
#endif
}

void SearchResultLocatorTest::testSearchResultLocatorForSdrObjects()
{
    if (!IsDefaultDPI())
        return;

    SwDoc* pDoc = createDoc("IndexingExport_Shapes.odt");
    CPPUNIT_ASSERT(pDoc);

    sw::search::SearchResultLocator aLocator(pDoc);
    std::vector<sw::search::SearchIndexData> aDataVector;
    aDataVector.emplace_back(sw::search::NodeType::CommonNode, 1, u"Circle");

    sw::search::LocationResult aResult = aLocator.find(aDataVector);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aResult.maRectangles.size());

    // skip asserting exact values for macOS and Windows because of
    // inconsistent results
#if !defined(_WIN32) && !defined(MACOSX)
    auto aRectangle = aResult.maRectangles[0];
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1478.0, aRectangle.getMinX(), 1e-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3223.0, aRectangle.getMinY(), 1e-4);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2059.0, aRectangle.getWidth(), 1e-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2059.0, aRectangle.getHeight(), 1e-4);
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(SearchResultLocatorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
