/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/ooxml/.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/writerfilter/qa/cppunittests/ooxml/data/")
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
    CPPUNIT_ASSERT(xTable->supportsService("com.sun.star.text.TextTable"));
    uno::Reference<lang::XServiceInfo> xParagraph(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParagraph->supportsService("com.sun.star.text.Paragraph"));
    CPPUNIT_ASSERT(!xParaEnum->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(Test, testRecursiveHeaderRels)
{
    // Given a document with self-referencing rels in a header/footer:
    loadFromFile(u"recursive_header_rels.docx");
    // It should not crash/hang on load
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
