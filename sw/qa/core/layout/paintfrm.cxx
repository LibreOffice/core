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
#include <svtools/DocumentToGraphicRenderer.hxx>

#include <docsh.hxx>
#include <unotxdoc.hxx>

namespace
{
/// Covers sw/source/core/layout/paintfrm.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/layout/data/"_ustr)
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
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//polyline[@style='solid']/point"_ostr);
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

CPPUNIT_TEST_FIXTURE(Test, testRTLBorderMerge)
{
    // Given a document with an RTL table:
    createSwDoc("rtl-table.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();

    // When rendering that document:
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();

    // Then make sure the 5 columns all have left and right vertical borders:
    MetafileXmlDump aDumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(aDumper, *xMetaFile);
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//polyline[@style='solid']/point"_ostr);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    int nVerticalBorders = 0;
    // Count the vertical borders:
    for (int i = 0; i < xmlXPathNodeSetGetLength(pXmlNodes); i += 2)
    {
        xmlNodePtr pStart = pXmlNodes->nodeTab[i];
        xmlNodePtr pEnd = pXmlNodes->nodeTab[i + 1];
        xmlChar* pStartY = xmlGetProp(pStart, BAD_CAST("y"));
        xmlChar* pEndY = xmlGetProp(pEnd, BAD_CAST("y"));
        sal_Int32 nStartY = o3tl::toInt32(reinterpret_cast<char const*>(pStartY));
        sal_Int32 nEndY = o3tl::toInt32(reinterpret_cast<char const*>(pEndY));
        if (nStartY == nEndY)
        {
            // Horizontal border.
            continue;
        }

        ++nVerticalBorders;
    }
    xmlXPathFreeObject(pXmlObj);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6
    // - Actual  : 4
    // i.e. the 2nd and 5th vertical border was missing.
    CPPUNIT_ASSERT_EQUAL(6, nVerticalBorders);
}

CPPUNIT_TEST_FIXTURE(Test, testSplitTableMergedBorder)
{
    // Given a document with a split table, first row in frame 1 has merged cells:
    createSwDoc("split-table-merged-border.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();

    // When rendering that document:
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();

    // Then make sure that the master table has a bottom border with the correct widths:
    MetafileXmlDump aDumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(aDumper, *xMetaFile);
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//polyline[@style='solid']/point"_ostr);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    std::set<int> aHorizontalBorderStarts;
    std::set<int> aHorizontalBorderEnds;
    // Collect the horizontal borders:
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

        xmlChar* pStartX = xmlGetProp(pStart, BAD_CAST("x"));
        xmlChar* pEndX = xmlGetProp(pEnd, BAD_CAST("x"));
        sal_Int32 nStartX = o3tl::toInt32(reinterpret_cast<char const*>(pStartX));
        sal_Int32 nEndX = o3tl::toInt32(reinterpret_cast<char const*>(pEndX));
        aHorizontalBorderStarts.insert(nStartX);
        aHorizontalBorderEnds.insert(nEndX);
    }
    xmlXPathFreeObject(pXmlObj);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aHorizontalBorderStarts.size());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 3
    // i.e. the frame 1 bottom border ended sooner than expected, resulting in a buggy, partial
    // bottom border.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aHorizontalBorderEnds.size());
}

CPPUNIT_TEST_FIXTURE(Test, testInlineEndnoteSeparatorPosition)
{
    // Given a document with a Word-style endnote separator:
    createSwDoc("inline-endnote-position.docx");
    SwDocShell* pDocShell = getSwDocShell();

    // When rendering that document:
    std::shared_ptr<GDIMetaFile> xMetaFile = pDocShell->GetPreviewMetaFile();

    // Then make sure the separator upper spacing is 60% of all space, matching Word:
    MetafileXmlDump aDumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(aDumper, *xMetaFile);
    auto nEndnoteSeparatorY = getXPath(pXmlDoc, "//polygon/point[1]"_ostr, "y"_ostr).toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2164
    // - Actual  : 2060
    // i.e. the upper spacing was too low.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2164), nEndnoteSeparatorY);

    // Also make sure the separator length is correct:
    auto nEndnoteSeparatorStart = getXPath(pXmlDoc, "//polygon/point[1]"_ostr, "x"_ostr).toInt32();
    auto nEndnoteSeparatorEnd = getXPath(pXmlDoc, "//polygon/point[2]"_ostr, "x"_ostr).toInt32();
    sal_Int32 nEndnoteSeparatorLength = nEndnoteSeparatorEnd - nEndnoteSeparatorStart;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2880
    // - Actual  : 2340
    // i.e. the separator wasn't 2 inches long, but was shorter vs Word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2880), nEndnoteSeparatorLength);
}

CPPUNIT_TEST_FIXTURE(Test, testEndnoteContSeparator)
{
    // Given a document with a Word-style endnote continuation separator:
    createSwDoc("endnote-cont-separator.docx");

    // When rendering page 2:
    sal_Int32 nPage = 2;
    DocumentToGraphicRenderer aRenderer(mxComponent, /*bSelectionOnly=*/false);
    Size aSize = aRenderer.getDocumentSizeInPixels(nPage);
    Graphic aGraphic = aRenderer.renderToGraphic(nPage, aSize, aSize, COL_WHITE,
                                                 /*bExtOutDevData=*/false);
    auto& xMetaFile = const_cast<GDIMetaFile&>(aGraphic.GetGDIMetaFile());

    // Then make sure the separator length is correct:
    MetafileXmlDump aDumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(aDumper, xMetaFile);
    auto nEndnoteSeparatorStart = getXPath(pXmlDoc, "//polygon/point[1]"_ostr, "x"_ostr).toInt32();
    auto nEndnoteSeparatorEnd = getXPath(pXmlDoc, "//polygon/point[2]"_ostr, "x"_ostr).toInt32();
    sal_Int32 nEndnoteSeparatorLength = nEndnoteSeparatorEnd - nEndnoteSeparatorStart;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 9360 (page print area width)
    // - Actual  : 2880 (2 inches)
    // i.e. the separator was too short vs Word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(9360), nEndnoteSeparatorLength);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
