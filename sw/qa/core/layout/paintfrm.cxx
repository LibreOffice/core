/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <o3tl/string_view.hxx>

#include <docsh.hxx>
#include <unotxdoc.hxx>

namespace
{
/// Covers sw/source/core/layout/paintfrm.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/layout/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testSplitTableBorder)
{
    // Given a document with a split table, table borders are defined, but cell borders are not:
    createSwDoc("split-table-border.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();

    // When rendering that document:
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();

    // Then make sure that the master table has a bottom border and the follow table has a top
    // border:
    MetafileXmlDump aDumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(aDumper, *xMetaFile);
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//polyline[@style='solid']/point");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    int nHorizontalBorders = 0;
    // Count the horizontal borders:
    for (int i = 0; i < xmlXPathNodeSetGetLength(pXmlNodes); i += 2)
    {
        xmlNodePtr pStart = pXmlNodes->nodeTab[i];
        xmlNodePtr pEnd = pXmlNodes->nodeTab[i + 1];
        xmlChar* pStartY = xmlGetProp(pStart, BAD_CAST("y"));
        xmlChar* pEndY = xmlGetProp(pEnd, BAD_CAST("y"));
        sal_Int32 nStartY = o3tl::toInt32(reinterpret_cast<char const*>(pStartY));
        sal_Int32 nEndY = o3tl::toInt32(reinterpret_cast<char const*>(pEndY));
        if (nStartY != nEndY)
        {
            // Vertical border.
            continue;
        }

        ++nHorizontalBorders;
    }
    xmlXPathFreeObject(pXmlObj);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : 2
    // i.e. the bottom border in the master table and the top border in the follow table were
    // missing.
    CPPUNIT_ASSERT_EQUAL(4, nHorizontalBorders);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
