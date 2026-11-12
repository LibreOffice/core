
/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextRange.hpp>

using namespace css;

namespace
{
class HwpImportTest : public SwModelTestBase
{
public:
    HwpImportTest()
        : SwModelTestBase(u"/sw/qa/extras/hwpimport/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(HwpImportTest, testTdf174425)
{
    createSwDoc("tdf174425_table2x2.hwp");

    // Check that Writer imported exactly one table.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<container::XNameAccess> xTables = xTablesSupplier->getTextTables();

    const uno::Sequence<OUString> aTableNames = xTables->getElementNames();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aTableNames.getLength());

    uno::Reference<text::XTextTable> xTable(xTables->getByName(aTableNames[0]),
                                            uno::UNO_QUERY_THROW);

    // Check the dimensions.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());

    // Check the contents of all four cells.
    const auto checkCell = [&xTable](const OUString& rName, const OUString& rExpected) {
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(rName), uno::UNO_QUERY_THROW);

        CPPUNIT_ASSERT_EQUAL(rExpected, xCell->getString());
    };

    checkCell(u"A1"_ustr, u"CELL-A1"_ustr);
    checkCell(u"B1"_ustr, u"CELL-B1"_ustr);
    checkCell(u"A2"_ustr, u"CELL-A2"_ustr);
    checkCell(u"B2"_ustr, u"CELL-B2"_ustr);
}
} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();
