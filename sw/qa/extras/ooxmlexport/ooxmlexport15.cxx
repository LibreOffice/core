/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <svx/svddef.hxx>
#include <svx/unoapi.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/svdobj.hxx>

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

DECLARE_OOXMLEXPORT_TEST(testGutterLeft, "gutter-left.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle;
    getStyles("PageStyles")->getByName("Standard") >>= xPageStyle;
    sal_Int32 nGutterMargin{};
    xPageStyle->getPropertyValue("GutterMargin") >>= nGutterMargin;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1270
    // - Actual  : 0
    // i.e. gutter margin was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), nGutterMargin);
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testGutterTop, "gutter-top.docx")
{
    xmlDocPtr pXmlSettings = parseExport("word/settings.xml");
    CPPUNIT_ASSERT(pXmlSettings);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. <w:gutterAtTop> was lost.
    assertXPath(pXmlSettings, "/w:settings/w:gutterAtTop", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf133473_shadowSize, "tdf133473.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    SdrObject* pObj(GetSdrObjectFromXShape(xShape));
    const SfxItemSet& rSet = pObj->GetMergedItemSet();
    sal_Int32 nSize1 = rSet.Get(SDRATTR_SHADOWSIZEX).GetValue();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 200000
    // - Actual  : 113386
    // I.e. Shadow size will be smaller than actual.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(200000), nSize1);
}

DECLARE_OOXMLEXPORT_TEST(testCommentDoneModel, "CommentDone.docx")
{
    css::uno::Reference<css::text::XTextFieldsSupplier> xTextFieldsSupplier(
        mxComponent, css::uno::UNO_QUERY_THROW);
    auto xFields(xTextFieldsSupplier->getTextFields()->createEnumeration());

    // First comment: resolved
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    css::uno::Any aComment = xFields->nextElement();
    css::uno::Reference<css::beans::XPropertySet> xComment(aComment, css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(true, xComment->getPropertyValue("Resolved").get<bool>());

    // Second comment: unresolved
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    aComment = xFields->nextElement();
    xComment.set(aComment, css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(false, xComment->getPropertyValue("Resolved").get<bool>());
}

DECLARE_OOXMLEXPORT_TEST(testTdf81507, "tdf81507.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return; // initial import, no futher checks

    // Ensure that we have <w:text w:multiLine="1"/>
    CPPUNIT_ASSERT_EQUAL(OUString("1"), getXPath(pXmlDoc, "/w:document/w:body/w:sdt[1]/w:sdtPr/w:text", "multiLine"));

    // Ensure that we have <w:text w:multiLine="0"/>
    CPPUNIT_ASSERT_EQUAL(OUString("0"), getXPath(pXmlDoc, "/w:document/w:body/w:sdt[2]/w:sdtPr/w:text", "multiLine"));

    // Ensure that we have <w:text/>
    getXPath(pXmlDoc, "/w:document/w:body/w:sdt[3]/w:sdtPr/w:text", "");

    // Ensure that we have no <w:text/> (not quite correct case, but to ensure import/export are okay)
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "/w:document/w:body/w:sdt[4]/w:sdtPr/w:text");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                           static_cast<sal_Int32>(xmlXPathNodeSetGetLength(pXmlObj->nodesetval)));
    xmlXPathFreeObject(pXmlObj);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
