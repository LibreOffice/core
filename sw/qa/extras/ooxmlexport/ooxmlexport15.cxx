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
    // Since non-DOCX formats ignores column breaks in non-column situations, don't export to docx.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf129452_excessBorder, "tdf129452_excessBorder.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY_THROW);

    // The outside border should not be applied on inside cells. The merge doesn't extend to the table bottom.
    // [Note: as a human, we would call this cell D3, but since row 4 hasn't been analyzed yet, it is considered column C.]
    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xTable->getCellByName("C3"), "BottomBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No bottom border on merged cell", sal_uInt32(0), aBorder.LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf134063, "tdf134063.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    xmlDocUniquePtr pDump = parseLayoutDump();

    // There are three tabs with default width
    CPPUNIT_ASSERT_EQUAL(sal_Int32(720), getXPath(pDump, "//page[1]/body/txt[1]/Text[1]", "nWidth").toInt32());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(720), getXPath(pDump, "//page[1]/body/txt[1]/Text[2]", "nWidth").toInt32());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(720), getXPath(pDump, "//page[1]/body/txt[1]/Text[3]", "nWidth").toInt32());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
