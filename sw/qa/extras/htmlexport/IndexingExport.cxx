/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <swmodeltestbase.hxx>

#include <test/htmltesttools.hxx>

class IndexingExportTest : public SwModelTestBase
{
public:
    IndexingExportTest()
        : SwModelTestBase("/sw/qa/extras/htmlexport/data/", "HTML (StarWriter)")
    {
    }

private:
    virtual std::unique_ptr<Resetter> preTest(const char*) override
    {
        setFilterOptions("IndexingOutput");
        return nullptr;
    }
};

#define DECLARE_INDEXINGEXPORT_TEST(TestName, filename)                                            \
    DECLARE_SW_EXPORT_TEST(TestName, filename, nullptr, IndexingExportTest)

DECLARE_INDEXINGEXPORT_TEST(testIndexingSimpleParagraph, "IndexingExport_VariousParagraphs.odt")
{
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/indexing", 1);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
