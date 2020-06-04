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
#include <com/sun/star/text/RelOrientation.hpp>

char const DATA_DIRECTORY[] = "/sw/qa/extras/ooxmlexport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text") {}

protected:
    /**
     * Denylist handling
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

DECLARE_OOXMLIMPORT_TEST(testTdf118701, "tdf118701.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("At least one paragraph is missing from the file!", 3, getParagraphs());
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
    // [Note: as humans, we would call this cell D3, but since row 4 hasn't been analyzed yet, it is considered column C.]
    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xTable->getCellByName("C3"), "BottomBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No bottom border on merged cell", sal_uInt32(0), aBorder.LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf131561_necessaryBorder, "tdf131561_necessaryBorder.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // Hand-crafted pre-emptive test to make sure borders aren't lost.
    // MS Word is interesting here. 2/3 of the merged cell has the right border, so what to do?
    table::BorderLine2 aBorderR = getProperty<table::BorderLine2>(xTable->getCellByName("A1"), "RightBorder");
    table::BorderLine2 aBorderL = getProperty<table::BorderLine2>(xTable->getCellByName("B1"), "LeftBorder");
    CPPUNIT_ASSERT_MESSAGE("Border between A1 and B1", (aBorderR.LineWidth + aBorderL.LineWidth) > 0);
    aBorderR = getProperty<table::BorderLine2>(xTable->getCellByName("A3"), "RightBorder");
    aBorderL = getProperty<table::BorderLine2>(xTable->getCellByName("B3"), "LeftBorder");
    CPPUNIT_ASSERT_MESSAGE("Border between A3 and B3", (aBorderR.LineWidth + aBorderL.LineWidth) > 0);
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

DECLARE_OOXMLEXPORT_TEST(testRelativeAnchorHeightFromBottomMarginHasFooter,
                         "tdf133070_testRelativeAnchorHeightFromBottomMarginHasFooter.docx")
{
    // TODO: fix export too
    if (mbExported)
        return;
    // tdf#133070 The height was set relative to page print area bottom,
    // but this was handled relative to page height.
    // Note: page print area bottom = margin + footer height.
    // In this case the footer exists.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    const sal_Int32 nAnchoredHeight
        = getXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1147), nAnchoredHeight);
}

DECLARE_OOXMLIMPORT_TEST(TestTdf132483, "tdf132483.docx")
{
    uno::Reference<beans::XPropertySet> xOLEProps(getShape(1), uno::UNO_QUERY_THROW);
    sal_Int16 nVRelPos = -1;
    sal_Int16 nHRelPos = -1;
    xOLEProps->getPropertyValue("VertOrientRelation") >>= nVRelPos;
    xOLEProps->getPropertyValue("HoriOrientRelation") >>= nHRelPos;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The OLE is shifted vertically",
        text::RelOrientation::PAGE_FRAME , nVRelPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The OLE is shifted horizontally",
        text::RelOrientation::PAGE_FRAME , nHRelPos);
}

DECLARE_OOXMLEXPORT_TEST(testRelativeAnchorHeightFromBottomMarginNoFooter,
                         "tdf133070_testRelativeAnchorHeightFromBottomMarginNoFooter.docx")
{
    // TODO: fix export too
    if (mbExported)
        return;
    // tdf#133070 The height was set relative to page print area bottom,
    // but this was handled relative to page height.
    // Note: page print area bottom = margin + footer height.
    // In this case the footer does not exist, so OpenDocument and OOXML margins are the same.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    const sal_Int32 nAnchoredHeight
        = getXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1147), nAnchoredHeight);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
