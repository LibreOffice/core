/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <o3tl/string_view.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr) {}
};

CPPUNIT_TEST_FIXTURE(Test, testTdf123621)
{
    loadAndSave("tdf123621.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionV/wp:posOffset"_ostr, u"1080135"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131540)
{
    loadAndReload("tdf131540.odt");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // There are 2 OLEs test if one of them moved on save:
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The shape1 moved on saving!", text::RelOrientation::PAGE_FRAME,
                                 getProperty<sal_Int16>(getShape(1), u"HoriOrientRelation"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The shape2 moved on saving!", text::RelOrientation::PAGE_FRAME,
                                 getProperty<sal_Int16>(getShape(2), u"HoriOrientRelation"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf131801, "tdf131801.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    xmlDocUniquePtr pDump = parseLayoutDump();
    // "1." is red
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, getXPath(pDump, "//page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"00ff0000"_ustr, getXPath(pDump, "//page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr, "color"_ostr));
    // "2." is red
    CPPUNIT_ASSERT_EQUAL(u"2."_ustr, getXPath(pDump, "//page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"00ff0000"_ustr, getXPath(pDump, "//page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr, "color"_ostr));
    // "3." is black
    CPPUNIT_ASSERT_EQUAL(u"3."_ustr, getXPath(pDump, "//page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"ffffffff"_ustr, getXPath(pDump, "//page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr, "color"_ostr));
    // "4." is black
    CPPUNIT_ASSERT_EQUAL(u"4."_ustr, getXPath(pDump, "//page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"ffffffff"_ustr, getXPath(pDump, "//page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr, "color"_ostr));
    // "5." is red
    CPPUNIT_ASSERT_EQUAL(u"5."_ustr, getXPath(pDump, "//page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"00ff0000"_ustr, getXPath(pDump, "//page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr, "color"_ostr));
    // "6." is red
    CPPUNIT_ASSERT_EQUAL(u"6."_ustr, getXPath(pDump, "//page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"00ff0000"_ustr, getXPath(pDump, "//page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr, "color"_ostr));
    // "7." is black
    CPPUNIT_ASSERT_EQUAL(u"7."_ustr, getXPath(pDump, "//page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"ffffffff"_ustr, getXPath(pDump, "//page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr, "color"_ostr));
    // "8." is black
    CPPUNIT_ASSERT_EQUAL(u"8."_ustr, getXPath(pDump, "//page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwFieldPortion[1]"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"ffffffff"_ustr, getXPath(pDump, "//page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwFieldPortion[1]/SwFont"_ostr, "color"_ostr));

    if (!isExported())
        return;
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:rStyle"_ostr,
        "val"_ostr, u"Emphasis"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:rStyle"_ostr,
        "val"_ostr, u"Emphasis"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[3]/w:pPr/w:rPr/w:rStyle"_ostr, 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[4]/w:pPr/w:rPr/w:rStyle"_ostr, 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[5]/w:pPr/w:rPr/w:rStyle"_ostr,
        "val"_ostr, u"Emphasis"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[5]/w:pPr/w:rPr/w:sz"_ostr,
        "val"_ostr, u"32"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:rPr/w:rStyle"_ostr,
        "val"_ostr, u"Emphasis"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:rPr/w:sz"_ostr,
        "val"_ostr, u"32"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[7]/w:pPr/w:rPr/w:rStyle"_ostr, 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[8]/w:pPr/w:rPr/w:rStyle"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133334_followPgStyle)
{
    loadAndReload("tdf133334_followPgStyle.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf137850_compat14ZOrder, "tdf137850_compat14ZOrder.docx")
{
    // The file contains 2 shapes which have a different value of behindDoc.
    // Test that the textbox is hidden behind the arrow (for Word <= 2010/compatibilityMode==14)
    uno::Reference<text::XText> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"2015"_ustr, xShape->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Textbox is in the background", false, getProperty<bool>(xShape, u"Opaque"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf137850_compat15ZOrder, "tdf137850_compat15ZOrder.docx")
{
    // The file contains 2 shapes which have a different value of behindDoc.
    // Test that the textbox is not hidden behind the arrow (for Word >= 2013/compatibilityMode==15)
    uno::Reference<text::XText> xShape(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"2015"_ustr, xShape->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Textbox is in the foreground", true, getProperty<bool>(xShape, u"Opaque"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf118701)
{
    loadAndSave("tdf118701.docx");
    // This was 6, related to moving inline images after the page breaks
    CPPUNIT_ASSERT_EQUAL(4, getPages());

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr[1]/w:numPr"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr[1]/w:numPr"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr[1]/w:numPr"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr[1]/w:numPr"_ostr, 1);

    // Keep numbering of the paragraph of the inline image
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:pPr[1]/w:numPr"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr[1]/w:numPr"_ostr, 1);
    // This was 0
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:pPr[1]/w:numPr"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf123388, "tdf123388.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // Tests new cell formula PRODUCT
    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"PRODUCT(<B2:B3>)"_ustr, xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"640"_ustr, xEnumerationAccess1->getPresentation(false).trim());
}

DECLARE_OOXMLEXPORT_TEST(testTdf123381, "tdf123381.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"INT(5.65)"_ustr, xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, xEnumerationAccess1->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"INT(<A1>)"_ustr, xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"6"_ustr, xEnumerationAccess2->getPresentation(false).trim());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123401)
{
    loadAndReload("tdf123401.fodt");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // Tests new cell formula AVERAGE
    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"AVERAGE(<A1:A2>)"_ustr, xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, xEnumerationAccess1->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"AVERAGE(<A1:A3>)"_ustr, xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, xEnumerationAccess2->getPresentation(false).trim());

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // MEAN converted to AVERAGE
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r[2]/w:instrText"_ostr, u" =AVERAGE(A1:A2)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[2]/w:instrText"_ostr, u" =AVERAGE(A1:A3)"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf116394, "tdf116394.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    uno::Reference<text::XTextField> xEnumerationAccess(xFields->nextElement(), uno::UNO_QUERY);

    // Without the fix in place, this test would have failed with
    // - Expected: ab=cd..
    // - Actual  : abcd..
    CPPUNIT_ASSERT_EQUAL(u"ab=cd.."_ustr, xEnumerationAccess->getPresentation(true).trim());

    if (!isExported())
        return;
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:instrText"_ostr, u" MERGEFIELD ab=cd "_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123356)
{
    loadAndReload("tdf123356.fodt");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // Tests new cell formula COUNT
    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"COUNT(<A1>)"_ustr, xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xEnumerationAccess1->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"COUNT(<A1:B2>)"_ustr, xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, xEnumerationAccess2->getPresentation(false).trim());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136404)
{
    loadAndReload("tdf136404.fodt");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // Ignore empty cells or cells with text content with new interoperability functions COUNT, AVERAGE and PRODUCT
    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"COUNT(<A1:F1>)"_ustr, xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, xEnumerationAccess1->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"AVERAGE(<B1:C1>)"_ustr, xEnumerationAccess2->getPresentation(true).trim());
    // This was 0
    CPPUNIT_ASSERT_EQUAL(u"** Expression is faulty **"_ustr, xEnumerationAccess2->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"AVERAGE(<B1>)"_ustr, xEnumerationAccess3->getPresentation(true).trim());
    // This was 0
    CPPUNIT_ASSERT_EQUAL(u"** Expression is faulty **"_ustr, xEnumerationAccess3->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess4(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"COUNT(<B1:C1>)"_ustr, xEnumerationAccess4->getPresentation(true).trim());
    // This was 2
    CPPUNIT_ASSERT_EQUAL(u"0"_ustr, xEnumerationAccess4->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess5(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"COUNT(<B1>)"_ustr, xEnumerationAccess5->getPresentation(true).trim());
    // This was 1
    CPPUNIT_ASSERT_EQUAL(u"0"_ustr, xEnumerationAccess5->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess6(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"PRODUCT(<A1:F1>)"_ustr, xEnumerationAccess6->getPresentation(true).trim());
    // This was 0
    CPPUNIT_ASSERT_EQUAL(u"60"_ustr, xEnumerationAccess6->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess7(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"AVERAGE(<A1:F1>)"_ustr, xEnumerationAccess7->getPresentation(true).trim());
    // This was 2
    CPPUNIT_ASSERT_EQUAL(u"8"_ustr, xEnumerationAccess7->getPresentation(false).trim());
}

DECLARE_OOXMLEXPORT_TEST(testTdf138739, "tdf138739.docx")
{
    uno::Reference<beans::XPropertySet> xParaProps(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Font type name does not match!", u"Comic Sans MS"_ustr,
                                 xParaProps->getPropertyValue(u"CharFontName"_ustr).get<OUString>());

    // tdf#148565: text at anchor point should be bold, Comic Sans MS font
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(getParagraph(5), 3), u"CharWeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123390)
{
    loadAndReload("tdf123390.fodt");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // Tests new cell formula SIGN
    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"SIGN(<A1>)"_ustr, xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"-1"_ustr, xEnumerationAccess1->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"SIGN(<C1>)"_ustr, xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xEnumerationAccess2->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"SIGN(<B1>)"_ustr, xEnumerationAccess3->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"0"_ustr, xEnumerationAccess3->getPresentation(false).trim());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123354)
{
    loadAndReload("tdf123354.fodt");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // Tests new cell formula SIGN
    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"ABS(<A1>)"_ustr, xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"10"_ustr, xEnumerationAccess1->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"ABS(<C1>)"_ustr, xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"10"_ustr, xEnumerationAccess2->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"ABS(<B1>)"_ustr, xEnumerationAccess3->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"0"_ustr, xEnumerationAccess3->getPresentation(false).trim());
}

DECLARE_OOXMLEXPORT_TEST(testTdf123355, "tdf123355.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // Tests conversion of range IDs ABOVE, BELOW, LEFT and RIGHT
    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    // Note: range ends at B4 here, which is a cell with text content
    CPPUNIT_ASSERT_EQUAL(u"average( <B2:B3> )"_ustr, xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"5,5"_ustr, xEnumerationAccess1->getPresentation(false).trim());

    // range ends at the end of the empty cells
    uno::Reference<text::XTextField> xEnumerationAccess6(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"SUM(<C6:A6>)"_ustr, xEnumerationAccess6->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"0"_ustr, xEnumerationAccess6->getPresentation(false).trim());

    // range starts at the first cell above D5
    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"AVERAGE(<D4:D1>)"_ustr, xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"5,33"_ustr, xEnumerationAccess2->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"AVERAGE(<C2:C1>)"_ustr, xEnumerationAccess3->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"3,5"_ustr, xEnumerationAccess3->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess4(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"AVERAGE(<D2:D2>)"_ustr, xEnumerationAccess4->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, xEnumerationAccess4->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess5(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"AVERAGE(<A2:A2>)"_ustr, xEnumerationAccess5->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, xEnumerationAccess5->getPresentation(false).trim());

    if (!isExported())
        return;
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // keep original formula IDs
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[2]/w:p/w:r[2]/w:instrText"_ostr, u" =average( below )"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:p/w:r[2]/w:instrText"_ostr, u" =AVERAGE(LEFT)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:p/w:r[2]/w:instrText"_ostr, u" =AVERAGE(RIGHT)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[3]/w:p/w:r[2]/w:instrText"_ostr, u" =AVERAGE(ABOVE)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =AVERAGE(ABOVE)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =SUM(LEFT)"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf123382, "tdf123382.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // Tests conversion of range IDs ABOVE, BELOW, LEFT and RIGHT
    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    // Note: range ends at B4 here, which is a cell with text content
    CPPUNIT_ASSERT_EQUAL(u"MAX(<B1:D1>)"_ustr, xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"12"_ustr, xEnumerationAccess1->getPresentation(false).trim());

    // range ends at the end of the empty cells
    uno::Reference<text::XTextField> xEnumerationAccess6(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"MAX(<C4:D4>)"_ustr, xEnumerationAccess6->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"9"_ustr, xEnumerationAccess6->getPresentation(false).trim());

    // range starts at the first cell above D5
    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"SUM(<B3:D3>)"_ustr, xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"30"_ustr, xEnumerationAccess2->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"MAX(<C2:A2>)"_ustr, xEnumerationAccess3->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, xEnumerationAccess3->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess4(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"MAX(<B2:D2>)"_ustr, xEnumerationAccess4->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, xEnumerationAccess4->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess5(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"MAX(<D2:D4>)"_ustr, xEnumerationAccess5->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"12"_ustr, xEnumerationAccess5->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess7(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"MAX(<B2:B4>)"_ustr, xEnumerationAccess7->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"10"_ustr, xEnumerationAccess7->getPresentation(false).trim());

    if (!isExported())
        return;
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // keep original formula IDs
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p/w:r[2]/w:instrText"_ostr, u" =MAX(RIGHT)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:p/w:r[2]/w:instrText"_ostr, u" =MAX(RIGHT)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[1]/w:p/w:r[2]/w:instrText"_ostr, u" =SUM(RIGHT)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[2]/w:p/w:r[2]/w:instrText"_ostr, u" =MAX(BELOW)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[2]/w:p/w:r[2]/w:instrText"_ostr, u" =MAX(RIGHT)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =MAX(BELOW)"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[4]/w:p/w:r[2]/w:instrText"_ostr, u" =MAX(LEFT)"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf122648, "tdf122648.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // table formula conversion worked only in the first table
    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"<A1>"_ustr, xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xEnumerationAccess1->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"SUM(<A1:B1>)"_ustr, xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, xEnumerationAccess2->getPresentation(false).trim());

    // These were <?> and SUM(<?:?>) with zero values
    uno::Reference<text::XTextField> xEnumerationAccess3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"<A1>"_ustr, xEnumerationAccess3->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xEnumerationAccess3->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess4(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"SUM(<A1:B1>)"_ustr, xEnumerationAccess4->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, xEnumerationAccess4->getPresentation(false).trim());

    if (!isExported())
        return;
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[2]/w:p/w:r[2]/w:instrText"_ostr, u" =A1"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[2]/w:tc[2]/w:p/w:r[2]/w:instrText"_ostr, u" =SUM(A1:B1)"_ustr);
    // These were =<?> and =SUM(<?:?>)
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl[2]/w:tr[1]/w:tc[2]/w:p/w:r[2]/w:instrText"_ostr, u" =A1"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl[2]/w:tr[2]/w:tc[2]/w:p/w:r[2]/w:instrText"_ostr, u" =SUM(A1:B1)"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf98000_changePageStyle)
{
    loadAndReload("tdf98000_changePageStyle.odt");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);

    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY_THROW);
    OUString sPageOneStyle = getProperty<OUString>( xCursor, u"PageStyleName"_ustr );

    xCursor->jumpToNextPage();
    OUString sPageTwoStyle = getProperty<OUString>( xCursor, u"PageStyleName"_ustr );
    CPPUNIT_ASSERT_MESSAGE("Different page1/page2 styles", sPageOneStyle != sPageTwoStyle);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf145998_unnecessaryPageStyles)
{
    loadAndReload("tdf145998_unnecessaryPageStyles.odt");

    // Sanity check - always good to test when dealing with page styles and breaks.
    CPPUNIT_ASSERT_EQUAL(5, getPages());

    // Page Style should be explicitly mentioned - otherwise it would be a "follow" style
    uno::Reference<beans::XPropertySet> xPara(getParagraph(2, u"2"_ustr), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(uno::Any() != xPara->getPropertyValue(u"PageDescName"_ustr));
    // CPPUNIT_ASSERT_EQUAL(OUString("First Page header"),
    //                      parseDump("/root/page[2]/header/txt"));

    // Page Style is converted into a page break instead. Still shows "first" header.
    xPara.set(getParagraph(3, u"3"_ustr), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(uno::Any(), xPara->getPropertyValue(u"PageDescName"_ustr));
    // CPPUNIT_ASSERT_EQUAL(OUString("Default page style - first page style"),
    //                      parseDump("/root/page[3]/header/txt"));
    CPPUNIT_ASSERT_EQUAL(OUString(), parseDump("/root/page[3]/footer/txt"_ostr));

    // Page Style is converted into a page break instead. Shows the "normal" header.
    xPara.set(getParagraph(5, u"4"_ustr), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(uno::Any(), xPara->getPropertyValue(u"PageDescName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Default page style"_ustr,
                         parseDump("/root/page[4]/header/txt"_ostr));

    // Page Style is retained (with wrong header) in order to preserve page re-numbering.
    xPara.set(getParagraph(7, u"1"_ustr), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(uno::Any() != xPara->getPropertyValue(u"PageDescName"_ustr));
    CPPUNIT_ASSERT_EQUAL(OUString(), parseDump("/root/page[5]/footer/txt"_ostr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136929_framesOfParagraph)
{
    loadAndReload("tdf136929_framesOfParagraph.odt");
    // Before this fix, the image was placed in the footer instead of in the text body - messing everything up.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 5, getPages() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Header2 text", u"* | *"_ustr, parseDump("/root/page[4]/footer/txt"_ostr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf136589_paraHadField, "tdf136589_paraHadField.docx")
{
    // The section break should not add an additional CR - which equals an empty page two.
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    //tdf#118711 - don't explicitly specify the default page style at the beginning of the document
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(uno::Any(), xPara->getPropertyValue(u"PageDescName"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133370_columnBreak)
{
    loadAndReload("tdf133370_columnBreak.odt");
    // Since non-DOCX formats ignores column breaks in non-column situations, don't export to docx.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134649_pageBreak)
{
    loadAndReload("tdf134649_pageBreak.fodt");
    // This was 1 (missing page break between tables).
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf135343_columnSectionBreak_c14, "tdf135343_columnSectionBreak_c14.docx")
{
    uno::Reference<beans::XPropertySet> xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(1), u"TextSection"_ustr);
    uno::Reference<text::XTextColumns> xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section one's columns", sal_Int16(2), xTextColumns->getColumnCount());

    // Old Word 2010 version - nextColumn breaks inside column sections are just treated as regular column breaks.
    //xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(12, "RTL 2"), "TextSection");
    //xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    //CPPUNIT_ASSERT_EQUAL_MESSAGE("Section four's columns", sal_Int16(3), xTextColumns->getColumnCount());
    //CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf135973)
{
    loadAndReload("tdf135973.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"2."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(6), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"2."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf135343_columnSectionBreak_c14v2, "tdf135343_columnSectionBreak_c14v2.docx")
{
    // In this Word 2010 v2, section three was changed to start with a nextColumn break instead of a continuous break.
    // The previous section has no columns, so this time start the columns on a new page.
    uno::Reference<beans::XPropertySet> xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(10, u""_ustr), u"TextSection"_ustr);
    uno::Reference<text::XTextColumns> xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section three's columns", sal_Int16(3), xTextColumns->getColumnCount());
    //CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf135343_columnSectionBreak_c12v3, "tdf135343_columnSectionBreak_c12v3.docx")
{
    // In this Word 20-3 v3, section one and two have different number of columns. It acts like a page break.
    uno::Reference<beans::XPropertySet> xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(1, u"Four columns,"_ustr), u"TextSection"_ustr);
    uno::Reference<text::XTextColumns> xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section one's columns", sal_Int16(4), xTextColumns->getColumnCount());

    xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(6, u"RTL 2"_ustr), u"TextSection"_ustr);
    xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section two's columns", sal_Int16(2), xTextColumns->getColumnCount());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf135343_columnSectionBreak_c15, "tdf135343_columnSectionBreak_c15.docx")
{
    // Word 2013+ version - nextColumn breaks inside column sections are always handled like nextPage breaks.
    uno::Reference<beans::XPropertySet> xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(12, u"RTL 2"_ustr), u"TextSection"_ustr);
    uno::Reference<text::XTextColumns> xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section four's columns", sal_Int16(3), xTextColumns->getColumnCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fits on two pages", 2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf121669_equalColumns, "tdf121669_equalColumns.docx")
{
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(1), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    // The property was ignored when deciding at export whether the columns were equal or not. Layout isn't reliable.
    CPPUNIT_ASSERT(getProperty<bool>(xTextColumns, u"IsAutomatic"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132149_pgBreak)
{
    loadAndReload("tdf132149_pgBreak.odt");
    // This 5 page document is designed to visually exaggerate the problems
    // of emulating LO's followed-by-page-style into MSWord's sections.
    // While much has been improved, there are extra pages present, which still need fixing.
    xmlDocUniquePtr pDump = parseLayoutDump();

    // No header on pages 1,2,3.
    assertXPath(pDump, "//page[2]/header"_ostr, 0);

    // Margins/page orientation between Right and Left page styles are different
    assertXPath(pDump, "//page[1]/infos/prtBounds"_ostr, "left"_ostr, u"1134"_ustr);  //Right page style
    assertXPath(pDump, "//page[2]/infos/prtBounds"_ostr, "left"_ostr, u"2268"_ustr);  //Left page style

    assertXPath(pDump, "//page[1]/infos/bounds"_ostr, "width"_ostr, u"8391"_ustr);  //landscape
    assertXPath(pDump, "//page[2]/infos/bounds"_ostr, "width"_ostr, u"5940"_ustr);  //portrait
    // This two-line 3rd page ought not to exist. DID YOU FIX ME? The real page 3 should be "8391" landscape.
    assertXPath(pDump, "//page[3]/infos/bounds"_ostr, "width"_ostr, u"5940"_ustr);
    // This really ought to be on odd page 3, but now it is on odd page 5.
    assertXPath(pDump, "//page[5]/infos/bounds"_ostr, "width"_ostr, u"8391"_ustr);
    assertXPath(pDump, "//page[5]/infos/prtBounds"_ostr, "right"_ostr, u"6122"_ustr);  //Left page style


    //Page style change here must not be lost. This SHOULD be on page 4, but sadly it is not.
    assertXPathContent(pDump, "//page[6]/header/txt"_ostr, u"First Page Style"_ustr);
    CPPUNIT_ASSERT(getXPathContent(pDump, "//page[6]/body/txt[1]"_ostr).startsWith("Lorem ipsum"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132149_pgBreakB)
{
    loadAndReload("tdf132149_pgBreakB.odt");
    // This 5 page document is designed to visually exaggerate the problems
    // of emulating LO's followed-by-page-style into MSWord's sections.
    xmlDocUniquePtr pDump = parseLayoutDump();

    //Sanity check to ensure the correct page is being tested. This SHOULD be on page 3, but sadly it is not.
    CPPUNIT_ASSERT(getXPathContent(pDump, "//page[5]/body/txt[1]"_ostr).startsWith("Lorem ipsum"));
    //Prior to this fix, the original alternation between portrait and landscape was completely lost.
    assertXPath(pDump, "//page[5]/infos/bounds"_ostr, "width"_ostr, u"8391"_ustr);  //landscape
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132149_pgBreak2)
{
    loadAndReload("tdf132149_pgBreak2.odt");
    // This 3 page document is designed to visually exaggerate the problems
    // of emulating LO's followed-by-page-style into MSWord's sections.

    // The only specified page style change should be between page 1 and 2.
    // When the first paragraph was split into 3, each paragraph specified a page break. The last was unnecessary.
    uno::Reference<beans::XPropertySet> xParaThree(getParagraph(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(uno::Any(), xParaThree->getPropertyValue(u"PageDescName"_ustr));
    // The ODT is only 2 paragraphs, but a hack to get the right page style breaks para1 into pieces.
    // This was 4 paragraphs - the unnecessary page break had hacked in another paragraph split.
    CPPUNIT_ASSERT_LESSEQUAL( 3, getParagraphs() );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136952_pgBreak3B)
{
    loadAndReload("tdf136952_pgBreak3B.odt");
    // This 4 page document is designed to visually exaggerate the problems
    // of emulating LO's followed-by-page-style into MSWord's sections.
    xmlDocUniquePtr pDump = parseLayoutDump();

    //page::breakAfter must not be lost.
    //Prior to this bug fix, the Lorem ipsum paragraph was in the middle of a portrait page, with no switch to landscape occurring.
    CPPUNIT_ASSERT(getXPathContent(pDump, "//page[3]/body/txt[1]"_ostr).startsWith("Lorem ipsum"));
    assertXPath(pDump, "//page[3]/infos/bounds"_ostr, "width"_ostr, u"8391"_ustr);  //landscape
}

DECLARE_OOXMLEXPORT_TEST(testTdf135949_anchoredBeforeBreak, "tdf135949_anchoredBeforeBreak.docx")
{
    xmlDocUniquePtr pDump = parseLayoutDump();
    //The picture was shown on page 2, because the empty paragraph before the page break was removed
    assertXPath(pDump, "//page[1]/body/txt/anchored/fly"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf129452_excessBorder, "tdf129452_excessBorder.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY_THROW);

    // The outside border should not be applied on inside cells. The merge doesn't extend to the table bottom.
    // [Note: as humans, we would call this cell D3, but since row 4 hasn't been analyzed yet, it is considered column C.]
    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xTable->getCellByName(u"C3"_ustr), u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No bottom border on merged cell", sal_uInt32(0), aBorder.LineWidth);

    // [Note: as humans, we would call this cell C3, but since row 4 hasn't been analyzed yet, it is considered column B.]
    aBorder = getProperty<table::BorderLine2>(xTable->getCellByName(u"B3"_ustr), u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No bottom border on merged cell", sal_uInt32(0), aBorder.LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf132898_missingBorder, "tdf132898_missingBorder.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // The bottom border from the last merged cell was not showing
    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xTable->getCellByName(u"A1"_ustr), u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Bottom border on merged cell", aBorder.LineWidth > 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf132898_extraBorder, "tdf132898_extraBorder.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // A border defined on an earlier merged cell was showing
    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xTable->getCellByName(u"C1"_ustr), u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No bottom border on merged cell", sal_uInt32(0), aBorder.LineWidth);
    // MS Word is interesting here. 2/3 of the merged cell has the right border, so what to do?
    aBorder = getProperty<table::BorderLine2>(xTable->getCellByName(u"C1"_ustr), u"RightBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No right border on merged cell", sal_uInt32(0), aBorder.LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf131561_necessaryBorder, "tdf131561_necessaryBorder.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // Hand-crafted pre-emptive test to make sure borders aren't lost.
    // MS Word is interesting here. 2/3 of the merged cell has the right border, so what to do?
    table::BorderLine2 aBorderR = getProperty<table::BorderLine2>(xTable->getCellByName(u"A1"_ustr), u"RightBorder"_ustr);
    table::BorderLine2 aBorderL = getProperty<table::BorderLine2>(xTable->getCellByName(u"B1"_ustr), u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Border between A1 and B1", (aBorderR.LineWidth + aBorderL.LineWidth) > 0);
    aBorderR = getProperty<table::BorderLine2>(xTable->getCellByName(u"A3"_ustr), u"RightBorder"_ustr);
    aBorderL = getProperty<table::BorderLine2>(xTable->getCellByName(u"B3"_ustr), u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Border between A3 and B3", (aBorderR.LineWidth + aBorderL.LineWidth) > 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf135655)
{
    loadAndSave("tdf135655.odt");
    const xmlDocUniquePtr pExpDoc = parseExport(u"word/document.xml"_ustr);
    const OUString sXFillColVal = getXPath(pExpDoc, "/w:document/w:body/w:p/w:r/w:object/v:shape"_ostr, "fillcolor"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"#00A933"_ustr, sXFillColVal);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf138374)
{
    loadAndSave("tdf138374.odt");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:r/w:pict/v:shape"_ostr, "fillcolor"_ostr, u"#ffd320"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:r/w:pict/v:shape"_ostr, "coordsize"_ostr, u"1315,6116"_ustr);
    if (IsDefaultDPI())
        assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:r/w:pict/v:shape"_ostr, "path"_ostr,
            u"m0,0l1314,0l1314,5914l416,5914l416,6115l106,5715l416,5415l416,5715l1014,5715l1014,224l0,224l0,16l0,0e"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:r/w:pict/v:shape"_ostr, "style"_ostr,
            u"position:absolute;margin-left:394.3pt;margin-top:204pt;width:37.2pt;height:173.3pt;mso-wrap-style:none;v-text-anchor:middle"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf134609_gridAfter, "tdf134609_gridAfter.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // Table borders (width 159) apply to edge cells, even in uneven cases caused by gridBefore/gridAfter,
    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xTable->getCellByName(u"A1"_ustr), u"RightBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Right border before gridAfter cells", aBorder.LineWidth > 0);
    aBorder = getProperty<table::BorderLine2>(xTable->getCellByName(u"E2"_ustr), u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Left edge border after gridBefore cells", aBorder.LineWidth > 100);
    aBorder = getProperty<table::BorderLine2>(xTable->getCellByName(u"E2"_ustr), u"TopBorder"_ustr);
    // but only for left/right borders, not top and bottom.
    // So somewhat inconsistently, gridBefore/After affects outside edges of columns, but not of rows.
    // insideH borders are width 53. (no insideV borders defined to emphasize missing edge borders)
    CPPUNIT_ASSERT_MESSAGE("Top border on 'inside' cell", aBorder.LineWidth > 0);
    CPPUNIT_ASSERT_MESSAGE("Top border is not an edge border", aBorder.LineWidth < 100);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf135329_lostImage)
{
    loadAndReload("tdf135329_lostImage.odt");
    // the character-anchored image was being skipped, since searchNext didn't notice it.
    uno::Reference<beans::XPropertySet> xImageProps(getShape(2), uno::UNO_QUERY_THROW);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136441_commentInFootnote)
{
    loadAndReload("tdf136441_commentInFootnote.odt");
    // failed to load without error if footnote contained a comment.
    // (MS Word's UI doesn't allow adding comments to a footnote.)
}

DECLARE_OOXMLEXPORT_TEST(testTdf137683_charHighlightTests, "tdf137683_charHighlightTests.docx")
{
    // Don't export unnecessary w:highlight="none" (Unnecessary one intentionally hand-added to original .docx)
    if (isExported())
    {
        xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);
        assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:rPr/w:highlight"_ostr, 0);
    }

    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(10), 2, u"no highlight"_ustr), uno::UNO_QUERY_THROW);
    // This test was failing with a cyan charHighlight of 65535 (0x00FFFF), instead of COL_TRANSPARENT (0xFFFFFFFF)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_AUTO), getProperty<sal_Int32>(xRun, u"CharHighlight"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf138345_charStyleHighlight, "tdf138345_charStyleHighlight.docx")
{
    // MS Word ignores the w:highlight setting in character styles. So shall we.
    // Without the fix, there would be an orange or yellow background on some words.
    const uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 2, u"orange background"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,u"CharHighlight"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,u"CharBackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125268)
{
    loadAndReload("tdf125268.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    const uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 1, u"Hello"_ustr), uno::UNO_QUERY);
    // Without the fix in place, this test would have failed with
    // - Expected: -1
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,u"CharHighlight"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_BLACK), getProperty<sal_Int32>(xRun,u"CharBackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf138345_numberingHighlight)
{
    loadAndSave("tdf138345_numberingHighlight.docx");
    // Before the fix, the highlight was completely lost.
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/numbering.xml"_ustr);
    if (pXmlStyles)
        assertXPath(pXmlStyles, "/w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:highlight"_ostr, "val"_ostr, u"red"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf134063, "tdf134063.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    xmlDocUniquePtr pDump = parseLayoutDump();

    // There are three tabs with default width
    CPPUNIT_ASSERT_EQUAL(sal_Int32(720), getXPath(pDump, "//page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFixPortion[1]"_ostr, "width"_ostr).toInt32());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(720), getXPath(pDump, "//page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFixPortion[2]"_ostr, "width"_ostr).toInt32());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(720), getXPath(pDump, "//page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFixPortion[3]"_ostr, "width"_ostr).toInt32());
}

DECLARE_OOXMLEXPORT_TEST(TestTdf135653, "tdf135653.docx")
{
    uno::Reference<beans::XPropertySet> xOLEProps(getShape(1), uno::UNO_QUERY_THROW);
    drawing::FillStyle nFillStyle = static_cast<drawing::FillStyle>(-1);
    xOLEProps->getPropertyValue(u"FillStyle"_ustr) >>= nFillStyle;
    Color aFillColor(COL_AUTO);
    xOLEProps->getPropertyValue(u"FillColor"_ustr) >>= aFillColor;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill style setting does not match!",
                                 drawing::FillStyle::FillStyle_SOLID, nFillStyle);
    Color aExpectedColor;
    aExpectedColor.SetRed(255);
    aExpectedColor.SetGreen(0);
    aExpectedColor.SetBlue(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("OLE bg color does not match!", aExpectedColor, aFillColor);
}

DECLARE_OOXMLEXPORT_TEST(testTdf135665, "tdf135665.docx")
{
    uno::Reference<beans::XPropertySet> xOLEProps1(getShape(1), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xOLEProps2(getShape(2), uno::UNO_QUERY_THROW);
    bool bSurroundContour1 = false;
    bool bSurroundContour2 = false;
    xOLEProps1->getPropertyValue(u"SurroundContour"_ustr) >>= bSurroundContour1;
    xOLEProps2->getPropertyValue(u"SurroundContour"_ustr) >>= bSurroundContour2;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("OLE tight wrap setting not imported correctly", true, bSurroundContour1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("OLE tight wrap setting not imported correctly", false, bSurroundContour2);
}

CPPUNIT_TEST_FIXTURE(Test, testAtPageShapeRelOrientation)
{
    loadAndSave("rotated_shape.fodt");
    // invalid combination of at-page anchor and horizontal-rel="paragraph"
    // caused relativeFrom="column" instead of relativeFrom="page"

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionH/wp:posOffset"_ostr, u"-480060"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionH"_ostr, "relativeFrom"_ostr, u"page"_ustr);
    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionV/wp:posOffset"_ostr, u"8147685"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionV"_ostr, "relativeFrom"_ostr, u"page"_ustr);

    // same for sw
    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor"
        "/wp:positionH/wp:posOffset"_ostr, u"720090"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor"
        "/wp:positionH"_ostr, "relativeFrom"_ostr, u"page"_ustr);
    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor"
        "/wp:positionV/wp:posOffset"_ostr, u"1080135"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor"
        "/wp:positionV"_ostr, "relativeFrom"_ostr, u"page"_ustr);

    // now test text rotation -> VML writing direction
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Fallback/w:pict/v:shape/v:textbox"_ostr, "style"_ostr, u"mso-layout-flow-alt:bottom-to-top"_ustr);
    // text wrap -> VML
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Fallback/w:pict/v:shape/w10:wrap"_ostr, "type"_ostr, u"none"_ustr);
    // vertical alignment -> VML
    OUString const style = getXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Fallback/w:pict/v:shape"_ostr, "style"_ostr);
    CPPUNIT_ASSERT(style.indexOf("v-text-anchor:middle") != -1);
}

CPPUNIT_TEST_FIXTURE(Test, testVMLallowincell)
{
    loadAndSave("shape-atpage-in-table.fodt");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // VML o:allowincell, apparently the default is "t"
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[1]/w:r/mc:AlternateContent[1]/mc:Fallback/w:pict/v:shape"_ostr, "allowincell"_ostr, u"f"_ustr);

    // DML layoutInCell
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[1]/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"_ostr, "layoutInCell"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testRelativeAnchorHeightFromBottomMarginHasFooter)
{
    loadAndSave("tdf133070_testRelativeAnchorHeightFromBottomMarginHasFooter.docx");
    // tdf#133070 The height was set relative to page print area bottom,
    // but this was handled relative to page height.
    // Note: page print area bottom = margin + footer height.
    // In this case the footer exists.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "height"_ostr, u"1147"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(TestTdf132483, "tdf132483.docx")
{
    uno::Reference<beans::XPropertySet> xOLEProps(getShape(1), uno::UNO_QUERY_THROW);
    sal_Int16 nVRelPos = -1;
    sal_Int16 nHRelPos = -1;
    xOLEProps->getPropertyValue(u"VertOrientRelation"_ustr) >>= nVRelPos;
    xOLEProps->getPropertyValue(u"HoriOrientRelation"_ustr) >>= nHRelPos;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The OLE is shifted vertically",
        text::RelOrientation::PAGE_FRAME , nVRelPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The OLE is shifted horizontally",
        text::RelOrientation::PAGE_FRAME , nHRelPos);
}

CPPUNIT_TEST_FIXTURE(Test, TestTdf143028)
{
    loadAndSave("fail_bracePair.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    auto pExportXml = parseExport(u"word/document.xml"_ustr);

    assertXPath(pExportXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                    "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:xfrm"_ostr);

}

CPPUNIT_TEST_FIXTURE(Test, testRelativeAnchorHeightFromBottomMarginNoFooter)
{
    loadAndSave("tdf133070_testRelativeAnchorHeightFromBottomMarginNoFooter.docx");
    // tdf#133070 The height was set relative to page print area bottom,
    // but this was handled relative to page height.
    // Note: page print area bottom = margin + footer height.
    // In this case the footer does not exist, so OpenDocument and OOXML margins are the same.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "height"_ostr, u"1147"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133702)
{
    loadAndSave("tdf133702.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf135667)
{
    loadAndSave("tdf135667.odt");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // This was missing.
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:r/w:object/v:shapetype"_ostr);

    // line settings
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:r/w:object/v:shape"_ostr, "stroked"_ostr, u"t"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:r/w:object/v:shape"_ostr, "strokecolor"_ostr, u"#FF0000"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:r/w:object/v:shape"_ostr, "strokeweight"_ostr, u"4pt"_ustr);

    // line type
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:r/w:object/v:shape/v:stroke"_ostr, "linestyle"_ostr, u"Single"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:r/w:object/v:shape/v:stroke"_ostr, "dashstyle"_ostr, u"Dash"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testImageSpaceSettings)
{
    loadAndSave("tdf135047_ImageSpaceSettings.fodt");
    // tdf#135047 The spaces of image were not saved.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:drawing/wp:anchor"_ostr, "distT"_ostr, u"90170"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:drawing/wp:anchor"_ostr, "distB"_ostr, u"90170"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:drawing/wp:anchor"_ostr, "distL"_ostr, u"90170"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:drawing/wp:anchor"_ostr, "distR"_ostr, u"90170"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf137295)
{
    loadAndReload("tdf137295.doc");
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without the fix in place, the test would have failed with
    // - Expected: 2
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

DECLARE_OOXMLEXPORT_TEST(testTdf135660, "tdf135660.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    const uno::Reference<drawing::XShape> xShape = getShape(1);
    const uno::Reference<beans::XPropertySet> xOLEProps(xShape, uno::UNO_QUERY_THROW);
    sal_Int32 nWrapDistanceLeft = -1;
    sal_Int32 nWrapDistanceRight = -1;
    sal_Int32 nWrapDistanceTop = -1;
    sal_Int32 nWrapDistanceBottom = -1;
    xOLEProps->getPropertyValue(u"LeftMargin"_ustr) >>= nWrapDistanceLeft;
    xOLEProps->getPropertyValue(u"RightMargin"_ustr) >>= nWrapDistanceRight;
    xOLEProps->getPropertyValue(u"TopMargin"_ustr) >>= nWrapDistanceTop;
    xOLEProps->getPropertyValue(u"BottomMargin"_ustr) >>= nWrapDistanceBottom;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Left wrap distance is wrong", static_cast<sal_Int32>(0), nWrapDistanceLeft);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Right wrap distance is wrong", static_cast<sal_Int32>(400), nWrapDistanceRight);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Top wrap distance is wrong", static_cast<sal_Int32>(300), nWrapDistanceTop);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bottom wrap distance is wrong", static_cast<sal_Int32>(199), nWrapDistanceBottom);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136814)
{
    loadAndSave("tdf136814.odt");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // Padding in this document is 0.10 cm which should translate to 3 pt (approx. 1.0583mm)
    assertXPath(pXmlDocument, "/w:document/w:body/w:sectPr/w:pgBorders/w:top"_ostr, "space"_ostr, u"3"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:sectPr/w:pgBorders/w:left"_ostr, "space"_ostr, u"3"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:sectPr/w:pgBorders/w:bottom"_ostr, "space"_ostr, u"3"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:sectPr/w:pgBorders/w:right"_ostr, "space"_ostr, u"3"_ustr);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
