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
}

CPPUNIT_TEST_SUITE_REGISTRATION(IndexingExportTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
