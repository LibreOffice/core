/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <fmtcntnt.hxx>
#include <ndtxt.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/ooxml/.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/writerfilter/ooxml/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFloatingTablesLost)
{
    // Given a document with 2 floating tables, the 2nd has an inner floating table as well:
    loadFromFile(u"floattable-tables-lost.docx");

    // When counting the created Writer tables:
    uno::Reference<text::XTextTablesSupplier> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextDocument->getTextTables(), uno::UNO_QUERY);

    // Then make sure that all 3 tables are imported:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3
    // - Actual  : 1
    // i.e. only the inner table was imported, the 2 others were lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTableLeak)
{
    // Given an outer table and 2 inner tables at B1 start:
    // When importing that document:
    loadFromFile(u"floattable-leak.docx");

    // Then make sure the body text only contains a table and an empty final paragraph:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<lang::XServiceInfo> xTable(xParaEnum->nextElement(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed, the document started with
    // a paragraph instead of a table.
    CPPUNIT_ASSERT(xTable->supportsService(u"com.sun.star.text.TextTable"_ustr));
    uno::Reference<lang::XServiceInfo> xParagraph(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParagraph->supportsService(u"com.sun.star.text.Paragraph"_ustr));
    CPPUNIT_ASSERT(!xParaEnum->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(Test, testRecursiveHeaderRels)
{
    // Given a document with self-referencing rels in a header/footer:
    loadFromFile(u"recursive_header_rels.docx");
    // It should not crash/hang on load

    // given an essentially corrupt document, which parses styles.xml multiple times,
    uno::Reference<style::XStyleFamiliesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies = xSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName(u"ParagraphStyles"_ustr), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xStandard(xStyleFamily->getByName(u"Standard"_ustr),
                                                  uno::UNO_QUERY_THROW);
    style::LineSpacing aLineSpacing;
    xStandard->getPropertyValue(u"ParaLineSpacing"_ustr) >>= aLineSpacing;
    // the proportional line spacing defined as default for the entire document should be 108%
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default para style has 1.08 line-spacing", sal_Int16(108),
                                 aLineSpacing.Height);
}

CPPUNIT_TEST_FIXTURE(Test, testNestedRuns)
{
    // Given a document with nested <w:r> in a shape text:
    // When importing that document:
    createSwDoc("nested-runs.docx");

    // Then make sure the shape text is not lost:
    SwDoc* pDoc = getSwDoc();
    sw::FrameFormats<sw::SpzFrameFormat*>& rFlys = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(!rFlys.empty());
    sw::SpzFrameFormat* pFly = rFlys[0];
    const SwNodeIndex* pFlyStartIndex = pFly->GetContent().GetContentIdx();
    CPPUNIT_ASSERT(pFlyStartIndex);
    SwNodeIndex aNodeIndex(*pFlyStartIndex);
    ++aNodeIndex;
    SwTextNode* pTextNode = aNodeIndex.GetNode().GetTextNode();
    CPPUNIT_ASSERT(pTextNode);
    // Without the accompanying fix in place, this test would have failed, the shape was empty.
    CPPUNIT_ASSERT_EQUAL(u"Test text box"_ustr, pTextNode->GetText());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
