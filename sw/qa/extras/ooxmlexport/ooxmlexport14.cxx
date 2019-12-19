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
#include <IDocumentSettingAccess.hxx>

#include <editsh.hxx>
#include <frmatr.hxx>
#include <tools/lineend.hxx>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/RelOrientation.hpp>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf87569v, "tdf87569_vml.docx")
{
    //the original tdf87569 sample has vml shapes...
    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    sal_Int16 nValue;
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf87569_vml: The Shape is not in the table!",
                                 text::RelOrientation::FRAME, nValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf87569d, "tdf87569_drawingml.docx")
{
    //if the original tdf87569 sample is upgraded it will have drawingml shapes...
    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    sal_Int16 nValue;
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf87569_drawingml: The Shape is not in the table!",
                                 text::RelOrientation::FRAME, nValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf120315, "tdf120315.docx")
{
    // tdf#120315 cells of the second column weren't vertically merged
    // because their horizontal positions are different a little bit
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
    CPPUNIT_ASSERT_EQUAL(getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(0), "TableColumnSeparators")[0]
                             .Position,
                         getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(1), "TableColumnSeparators")[2]
                             .Position);
}

DECLARE_OOXMLEXPORT_TEST(testTdf108350_noFontdefaults, "tdf108350_noFontdefaults.docx")
{
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference< beans::XPropertySet > xStyleProps(paragraphStyles->getByName("NoParent"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Times New Roman"), getProperty<OUString>(xStyleProps, "CharFontName"));
    //CPPUNIT_ASSERT_EQUAL_MESSAGE("Font size", 10.f, getProperty<float>(xStyleProps, "CharHeight"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf125038, "tdf125038.docx")
{
    OUString aActual = getParagraph(1)->getString();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: phone:...
    // - Actual  : result1result2phone:...
    // i.e. the result if the inner MERGEFIELD fields ended up in the body text.
    CPPUNIT_ASSERT_EQUAL(OUString("phone: \t1234567890"), aActual);
}

DECLARE_OOXMLIMPORT_TEST(testTdf124986, "tdf124986.docx")
{
    // Load a document with SET fields, where the SET fields contain leading/trailing quotation marks and spaces.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    while (xFields->hasMoreElements())
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xFields->nextElement(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xServiceInfo, uno::UNO_QUERY);
        OUString aValue;
        if (xServiceInfo->supportsService("com.sun.star.text.TextField.SetExpression"))
        {
            xPropertySet->getPropertyValue("Content") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("demo"), aValue);
        }
    }
}

DECLARE_OOXMLIMPORT_TEST(testTdf125038b, "tdf125038b.docx")
{
    // Load a document with an IF field, where the IF field command contains a paragraph break.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphAccess->createEnumeration();
    CPPUNIT_ASSERT(xParagraphs->hasMoreElements());
    uno::Reference<text::XTextRange> xParagraph(xParagraphs->nextElement(), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: phone: 1234
    // - Actual  :
    // i.e. the first paragraph was empty and the second paragraph had the content.
    CPPUNIT_ASSERT_EQUAL(OUString("phone: 1234"), xParagraph->getString());
    CPPUNIT_ASSERT(xParagraphs->hasMoreElements());
    xParagraphs->nextElement();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: !xParagraphs->hasMoreElements()
    // i.e. the document had 3 paragraphs, while only 2 was expected.
    CPPUNIT_ASSERT(!xParagraphs->hasMoreElements());
}

DECLARE_OOXMLIMPORT_TEST(testTdf125038c, "tdf125038c.docx")
{
    OUString aActual = getParagraph(1)->getString();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: email: test@test.test
    // - Actual  : email:
    // I.e. the result of the MERGEFIELD field inside an IF field was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("email: test@test.test"), aActual);
}

DECLARE_OOXMLEXPORT_TEST(testTdf121661, "tdf121661.docx")
{
    xmlDocPtr pXmlSettings = parseExport("word/settings.xml");
    if (!pXmlSettings)
        return;
    assertXPath(pXmlSettings, "/w:settings/w:hyphenationZone", "val", "851");
}

DECLARE_OOXMLEXPORT_TEST(testTdf121658, "tdf121658.docx")
{
    xmlDocPtr pXmlSettings = parseExport("word/settings.xml");
    if (!pXmlSettings)
        return;
    assertXPath(pXmlSettings, "/w:settings/w:doNotHyphenateCaps");
}

DECLARE_OOXMLEXPORT_TEST(testTdf124367, "tdf124367.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
    // it was 2761 at the first import, and 2760 at the second import, due to incorrect rounding
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2762),
                         getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(2), "TableColumnSeparators")[0]
                             .Position);
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf128820, "tdf128820.fodt")
{
    // Import of exported DOCX failed because of wrong namespace used for wsp element
    // Now test the exported XML, in case we stop failing opening invalid files
    xmlDocPtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    // The parent wpg:wgp element has three children: wpg:cNvGrpSpPr, wpg:grpSpPr, and wpg:wsp
    // (if we start legitimately exporting additional children, this needs to be adjusted to check
    // all those, to make sure we don't export wrong elements).
    assertXPathChildren(pXml,
                        "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                        "wp:inline/a:graphic/a:graphicData/wpg:wgp",
                        3);
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wpg:cNvGrpSpPr");
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wpg:grpSpPr");
    // This one was pic:wsp instead of wps:wsp
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wps:wsp");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf128889, "tdf128889.fodt")
{
    xmlDocPtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    // There was an w:r (with w:br) as an invalid child of first paragraph's w:pPr
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:pPr/w:r", 0);
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:r", 2);
    // Check that the break is in proper - last - position
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:r[2]/w:br", "type", "page");
}

DECLARE_OOXMLEXPORT_TEST(testTdf129353, "tdf129353.docx")
{
    CPPUNIT_ASSERT_EQUAL(8, getParagraphs());
    getParagraph(1, "(Verne, 1870)");
    getParagraph(2, "Bibliography");
    getParagraph(4, "Christie, A. (1922). The Secret Adversary. ");
    CPPUNIT_ASSERT_EQUAL(OUString(), getParagraph(8)->getString());

    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes = xIndexSupplier->getDocumentIndexes();
    uno::Reference<text::XDocumentIndex> xIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange = xIndex->getAnchor();
    uno::Reference<text::XText> xText = xTextRange->getText();
    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
    xTextCursor->gotoRange(xTextRange->getStart(), false);
    xTextCursor->gotoRange(xTextRange->getEnd(), true);
    OUString aIndexString(convertLineEnd(xTextCursor->getString(), LineEnd::LINEEND_LF));

    // Check that all the pre-rendered entries are correct, including trailing spaces
    CPPUNIT_ASSERT_EQUAL(OUString("\n" // starting with an empty paragraph
                                  "Christie, A. (1922). The Secret Adversary. \n"
                                  "\n"
                                  "Verne, J. G. (1870). Twenty Thousand Leagues Under the Sea. \n"
                                  ""), // ending with an empty paragraph
                         aIndexString);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
