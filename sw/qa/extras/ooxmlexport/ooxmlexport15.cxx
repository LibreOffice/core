/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextField.hpp>

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

DECLARE_OOXMLIMPORT_TEST(testTdf131801, "tdf131801.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    xmlDocPtr pDump = parseLayoutDump();
    // "1." is red
    CPPUNIT_ASSERT_EQUAL(OUString("1."), getXPath(pDump, "//page[1]/body/txt[1]/Special", "rText"));
    CPPUNIT_ASSERT_EQUAL(OUString("00ff0000"), getXPath(pDump, "//page[1]/body/txt[1]/Special/SwFont", "color"));
    // "2." is red
    CPPUNIT_ASSERT_EQUAL(OUString("2."), getXPath(pDump, "//page[1]/body/txt[2]/Special", "rText"));
    CPPUNIT_ASSERT_EQUAL(OUString("00ff0000"), getXPath(pDump, "//page[1]/body/txt[2]/Special/SwFont", "color"));
    // "3." is black
    CPPUNIT_ASSERT_EQUAL(OUString("3."), getXPath(pDump, "//page[1]/body/txt[3]/Special", "rText"));
    CPPUNIT_ASSERT_EQUAL(OUString("ffffffff"), getXPath(pDump, "//page[1]/body/txt[3]/Special/SwFont", "color"));
    // "4." is black
    CPPUNIT_ASSERT_EQUAL(OUString("4."), getXPath(pDump, "//page[1]/body/txt[4]/Special", "rText"));
    CPPUNIT_ASSERT_EQUAL(OUString("ffffffff"), getXPath(pDump, "//page[1]/body/txt[4]/Special/SwFont", "color"));
    // "5." is red
    CPPUNIT_ASSERT_EQUAL(OUString("5."), getXPath(pDump, "//page[1]/body/txt[5]/Special", "rText"));
    CPPUNIT_ASSERT_EQUAL(OUString("00ff0000"), getXPath(pDump, "//page[1]/body/txt[5]/Special/SwFont", "color"));
    // "6." is red
    CPPUNIT_ASSERT_EQUAL(OUString("6."), getXPath(pDump, "//page[1]/body/txt[6]/Special", "rText"));
    CPPUNIT_ASSERT_EQUAL(OUString("00ff0000"), getXPath(pDump, "//page[1]/body/txt[6]/Special/SwFont", "color"));
    // "7." is black
    CPPUNIT_ASSERT_EQUAL(OUString("7."), getXPath(pDump, "//page[1]/body/txt[7]/Special", "rText"));
    CPPUNIT_ASSERT_EQUAL(OUString("ffffffff"), getXPath(pDump, "//page[1]/body/txt[7]/Special/SwFont", "color"));
    // "8." is black
    CPPUNIT_ASSERT_EQUAL(OUString("8."), getXPath(pDump, "//page[1]/body/txt[8]/Special[1]", "rText"));
    CPPUNIT_ASSERT_EQUAL(OUString("ffffffff"), getXPath(pDump, "//page[1]/body/txt[8]/Special[1]/SwFont", "color"));

    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:rStyle",
        "val", "Emphasis");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:rStyle",
        "val", "Emphasis");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[3]/w:pPr/w:rPr/w:rStyle", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[4]/w:pPr/w:rPr/w:rStyle", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[5]/w:pPr/w:rPr/w:rStyle",
        "val", "Emphasis");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[5]/w:pPr/w:rPr/w:sz",
        "val", "32");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:rPr/w:rStyle",
        "val", "Emphasis");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:rPr/w:sz",
        "val", "32");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[7]/w:pPr/w:rPr/w:rStyle", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[8]/w:pPr/w:rPr/w:rStyle", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf122648, "tdf122648.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // table formula conversion worked only in the first table
    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("<A1>"), xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("1"), xEnumerationAccess1->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("SUM(<A1:B1>)"), xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("2"), xEnumerationAccess2->getPresentation(false).trim());

    // These were <?> and SUM(<?:?>) with zero values
    uno::Reference<text::XTextField> xEnumerationAccess3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("<A1>"), xEnumerationAccess3->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("1"), xEnumerationAccess3->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess4(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("SUM(<A1:B1>)"), xEnumerationAccess4->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("2"), xEnumerationAccess4->getPresentation(false).trim());

    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[2]/w:p/w:r[2]/w:instrText", " =  A1");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[2]/w:tc[2]/w:p/w:r[2]/w:instrText", " =  SUM(A1:B1)");
    // These were =<?> and =SUM(<?:?>)
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl[2]/w:tr[1]/w:tc[2]/w:p/w:r[2]/w:instrText", " =  A1");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl[2]/w:tr[2]/w:tc[2]/w:p/w:r[2]/w:instrText", " =  SUM(A1:B1)");
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

DECLARE_OOXMLEXPORT_TEST(testTdf138953, "croppedAndRotated.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Make sure the rotation is exported correctly, and size not distorted
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_EQUAL(27000.0, getProperty<double>(xShape, "RotateAngle"));
    auto frameRect = getProperty<css::awt::Rectangle>(xShape, "FrameRect");
    // Before the fix, original object size (i.e., before cropping) was written to spPr in OOXML,
    // and the resulting object size was much larger than should be.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12961), frameRect.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8664), frameRect.Width);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
