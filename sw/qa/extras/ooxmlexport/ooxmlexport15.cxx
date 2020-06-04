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

char const DATA_DIRECTORY[] = "/sw/qa/extras/ooxmlexport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf133334_followPgStyle, "tdf133334_followPgStyle.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf133370_columnBreak, "tdf133370_columnBreak.odt")
{
    // Since non-DOCX formats ignores column breaks in non-column situtations, don't export to docx.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf132898_missingBorder, "tdf132898_missingBorder.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // The bottom border from the last merged cell was not showing
    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xTable->getCellByName("A1"), "BottomBorder");
    CPPUNIT_ASSERT_MESSAGE("Bottom border on merged cell", aBorder.LineWidth > 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf132898_extraBorder, "tdf132898_extraBorder.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // A border defined on an earlier merged cell was showing
    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xTable->getCellByName("C1"), "BottomBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No bottom border on merged cell", sal_uInt32(0), aBorder.LineWidth);
    // MS Word is interesting here. 2/3 of the merged cell has the right border, so what to do?
    aBorder = getProperty<table::BorderLine2>(xTable->getCellByName("C1"), "RightBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No right border on merged cell", sal_uInt32(0), aBorder.LineWidth);
}


CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
