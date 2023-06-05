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
    loadFromURL(u"floattable-tables-lost.docx");

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
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
