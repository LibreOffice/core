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
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextField.hpp>

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

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf123621, "tdf123621.docx")
{
    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");

    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionV/wp:posOffset", "1080135");
}


DECLARE_OOXMLEXPORT_TEST(testTdf133334_followPgStyle, "tdf133334_followPgStyle.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf118701, "tdf118701.docx")
{
    // This was 6, related to moving inline images after the page breaks
    CPPUNIT_ASSERT_EQUAL(4, getPages());

    xmlDocUniquePtr pXmlDoc = parseExport();

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr[1]/w:numPr", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr[1]/w:numPr", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr[1]/w:numPr", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr[1]/w:numPr", 1);

    // Keep numbering of the paragraph of the inline image
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:pPr[1]/w:numPr", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr[1]/w:numPr", 1);
    // This was 0
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:pPr[1]/w:numPr", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf123354, "tdf123354.docx")
{
    // Tests cell formula ABS(x) to SQRT(x POW 2) conversion
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("SQRT((-234) POW 2)"), xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("234"), xEnumerationAccess1->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("SQRT((SUM(<A1:B1>)-SUM(<A2>)*2) POW 2)"), xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("700035"), xEnumerationAccess2->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("SQRT((<A1>) POW 2)"), xEnumerationAccess3->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("233423"), xEnumerationAccess3->getPresentation(false).trim());
}

DECLARE_OOXMLEXPORT_TEST(testTdf123390, "tdf123390.docx")
{
    // Tests cell formula SIGN(x) to (0 < x) - (x < 0) conversion
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("((0 L (-11)) - ((-11) L 0))"), xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("-1"), xEnumerationAccess1->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess4(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("((0 L (<B2>)) - ((<B2>) L 0))"), xEnumerationAccess4->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("0"), xEnumerationAccess4->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("((0 L (0)) - ((0) L 0))"), xEnumerationAccess3->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("0"), xEnumerationAccess3->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("((0 L (<B1>)) - ((<B1>) L 0))"), xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("1"), xEnumerationAccess2->getPresentation(false).trim());
}

DECLARE_OOXMLEXPORT_TEST(testTdf123356, "tdf123356.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("-11"), xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("-11"), xEnumerationAccess1->getPresentation(false).trim());

    // Tests cell formula COUNT(x) to SUM(x)/MEAN(x) conversion
    uno::Reference<text::XTextField> xEnumerationAccess4(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("(SUM(<A1:C1>|<A2:B2>)/MEAN(<A1:C1>|<A2:B2>))"), xEnumerationAccess4->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("5"), xEnumerationAccess4->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("(SUM(<A1:C1>)/MEAN(<A1:C1>))"), xEnumerationAccess3->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("3"), xEnumerationAccess3->getPresentation(false).trim());
}

DECLARE_OOXMLEXPORT_TEST(testTdf123388, "tdf123388.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // Tests new cell formula PRODUCT
    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("PRODUCT(<B2:B3>)"), xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("640"), xEnumerationAccess1->getPresentation(false).trim());
}

DECLARE_OOXMLEXPORT_TEST(testTdf123401, "tdf123401.fodt")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // Tests new cell formula AVERAGE
    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("AVERAGE(<A1:A2>)"), xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("3"), xEnumerationAccess1->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("AVERAGE(<A1:A3>)"), xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("3"), xEnumerationAccess2->getPresentation(false).trim());

    xmlDocUniquePtr pXmlDoc = parseExport();

    // MEAN converted to AVERAGE
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r[2]/w:instrText", " = AVERAGE(A1:A2)");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[2]/w:instrText", " = AVERAGE(A1:A3)");
}

DECLARE_OOXMLEXPORT_TEST(testTdf98000_changePageStyle, "tdf98000_changePageStyle.odt")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);

    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY_THROW);
    OUString sPageOneStyle = getProperty<OUString>( xCursor, "PageStyleName" );

    xCursor->jumpToNextPage();
    OUString sPageTwoStyle = getProperty<OUString>( xCursor, "PageStyleName" );
    CPPUNIT_ASSERT_MESSAGE("Different page1/page2 styles", sPageOneStyle != sPageTwoStyle);
}

DECLARE_OOXMLEXPORT_TEST(testTdf135216_evenOddFooter, "tdf135216_evenOddFooter.odt")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);

    // get LO page style for the first page (even page #2)
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<style::XStyle> xPageStyle(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);

    xCursor->jumpToFirstPage();  // Even/Left page #2
    uno::Reference<text::XText> xFooter = getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextLeft");
    CPPUNIT_ASSERT_EQUAL(OUString("even page"), xFooter->getString());

    xCursor->jumpToNextPage();
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    xFooter.set(getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextRight"));
    CPPUNIT_ASSERT_EQUAL(OUString("odd page - first footer"), xFooter->getString());

    xCursor->jumpToNextPage();
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    xFooter.set(getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextLeft"));
    CPPUNIT_ASSERT_EQUAL(OUString("even page"), xFooter->getString());

    // The contents of paragraph 2 should be the page number (2) located on page 1.
    getParagraph(2, "2");
}

DECLARE_OOXMLEXPORT_TEST(testTdf133370_columnBreak, "tdf133370_columnBreak.odt")
{
    // Since non-DOCX formats ignores column breaks in non-column situations, don't export to docx.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf135343_columnSectionBreak_c14, "tdf135343_columnSectionBreak_c14.docx")
{
    uno::Reference<beans::XPropertySet> xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(1), "TextSection");
    uno::Reference<text::XTextColumns> xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section one's columns", sal_Int16(2), xTextColumns->getColumnCount());

    // Old Word 2010 version - nextColumn breaks inside column sections are just treated as regular column breaks.
    //xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(12, "RTL 2"), "TextSection");
    //xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    //CPPUNIT_ASSERT_EQUAL_MESSAGE("Section four's columns", sal_Int16(3), xTextColumns->getColumnCount());
    //CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf135973, "tdf135973.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("2."), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(6), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("2."), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf135343_columnSectionBreak_c14v2, "tdf135343_columnSectionBreak_c14v2.docx")
{
    // In this Word 2010 v2, section three was changed to start with a nextColumn break instead of a continuous break.
    // The previous section has no columns, so this time start the columns on a new page.
    uno::Reference<beans::XPropertySet> xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(10, ""), "TextSection");
    uno::Reference<text::XTextColumns> xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section three's columns", sal_Int16(3), xTextColumns->getColumnCount());
    //CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf135343_columnSectionBreak_c12v3, "tdf135343_columnSectionBreak_c12v3.docx")
{
    // In this Word 20-3 v3, section one and two have different number of columns. It acts like a page break.
    uno::Reference<beans::XPropertySet> xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(1, "Four columns,"), "TextSection");
    uno::Reference<text::XTextColumns> xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section one's columns", sal_Int16(4), xTextColumns->getColumnCount());

    xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(6, "RTL 2"), "TextSection");
    xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section two's columns", sal_Int16(2), xTextColumns->getColumnCount());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf135343_columnSectionBreak_c15, "tdf135343_columnSectionBreak_c15.docx")
{
    // Word 2013+ version - nextColumn breaks inside column sections are always handled like nextPage breaks.
    uno::Reference<beans::XPropertySet> xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(12, "RTL 2"), "TextSection");
    uno::Reference<text::XTextColumns> xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section four's columns", sal_Int16(3), xTextColumns->getColumnCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fits on two pages", 2, getPages());
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

    // [Note: as humans, we would call this cell C3, but since row 4 hasn't been analyzed yet, it is considered column B.]
    aBorder = getProperty<table::BorderLine2>(xTable->getCellByName("B3"), "BottomBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No bottom border on merged cell", sal_uInt32(0), aBorder.LineWidth);
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

DECLARE_OOXMLEXPORT_TEST(testTdf134609_gridAfter, "tdf134609_gridAfter.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // Table borders (width 159) apply to edge cells, even in uneven cases caused by gridBefore/gridAfter,
    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xTable->getCellByName("A1"), "RightBorder");
    CPPUNIT_ASSERT_MESSAGE("Right border before gridAfter cells", aBorder.LineWidth > 0);
    aBorder = getProperty<table::BorderLine2>(xTable->getCellByName("E2"), "LeftBorder");
    CPPUNIT_ASSERT_MESSAGE("Left edge border after gridBefore cells", aBorder.LineWidth > 100);
    aBorder = getProperty<table::BorderLine2>(xTable->getCellByName("E2"), "TopBorder");
    // but only for left/right borders, not top and bottom.
    // So somewhat inconsistently, gridBefore/After affects outside edges of columns, but not of rows.
    // insideH borders are width 53. (no insideV borders defined to emphasize missing edge borders)
    CPPUNIT_ASSERT_MESSAGE("Top border on 'inside' cell", aBorder.LineWidth > 0);
    CPPUNIT_ASSERT_MESSAGE("Top border is not an edge border", aBorder.LineWidth < 100);
}

DECLARE_OOXMLEXPORT_TEST(testTdf135329_lostImage, "tdf135329_lostImage.odt")
{
    // the character-anchored image was being skipped, since searchNext didn't notice it.
    uno::Reference<beans::XPropertySet> xImageProps(getShape(2), uno::UNO_QUERY_THROW);
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

DECLARE_OOXMLEXPORT_TEST(testAtPageShapeRelOrientation, "rotated_shape.fodt")
{
    // invalid combination of at-page anchor and horizontal-rel="paragraph"
    // caused relativeFrom="column" instead of relativeFrom="page"

    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionH/wp:posOffset", "-480060");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionH", "relativeFrom", "page");
    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionV/wp:posOffset", "8147685");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionV", "relativeFrom", "page");

    // same for sw
    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor"
        "/wp:positionH/wp:posOffset", "720090");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor"
        "/wp:positionH", "relativeFrom", "page");
    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor"
        "/wp:positionV/wp:posOffset", "1080135");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor"
        "/wp:positionV", "relativeFrom", "page");

    // now test text rotation -> VML writing direction
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Fallback/w:pict/v:shape/v:textbox", "style", "mso-layout-flow-alt:bottom-to-top");
    // text wrap -> VML
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Fallback/w:pict/v:shape/w10:wrap", "type", "none");
    // vertical alignment -> VML
    OUString const style = getXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Fallback/w:pict/v:shape", "style");
    CPPUNIT_ASSERT(style.indexOf("v-text-anchor:middle") != -1);
}

DECLARE_OOXMLEXPORT_TEST(testRelativeAnchorHeightFromBottomMarginHasFooter,
                         "tdf133070_testRelativeAnchorHeightFromBottomMarginHasFooter.docx")
{
    // tdf#133070 The height was set relative to page print area bottom,
    // but this was handled relative to page height.
    // Note: page print area bottom = margin + footer height.
    // In this case the footer exists.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "height", "1147");
}

DECLARE_OOXMLIMPORT_TEST(TestTdf112342, "tdf112342.docx")
{
    //Get the last para
    uno::Reference<text::XTextRange> xPara = getParagraph(3);
    auto xCur = xPara->getText()->createTextCursor();
    //Go to the end of it
    xCur->gotoEnd(false);
    //And let's remove the last 2 chars (the last para with its char).
    xCur->goLeft(2, true);
    xCur->setString("");

    //If the second paragraph on the second page, this will be passed.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Page break does not match", 2, getPages());
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
    // tdf#133070 The height was set relative to page print area bottom,
    // but this was handled relative to page height.
    // Note: page print area bottom = margin + footer height.
    // In this case the footer does not exist, so OpenDocument and OOXML margins are the same.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "height", "1147");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf133702, "tdf133702.docx")
{
    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
