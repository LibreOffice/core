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
#include <oox/drawingml/drawingmltypes.hxx>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>

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

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf128207, "tdf128207.docx")
{
    //There was the charts on each other, because their horizontal and vertical position was 0!
    xmlDocPtr p_XmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPathContent(p_XmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:drawing/wp:anchor/wp:positionH/wp:posOffset", "4445");
}

DECLARE_OOXMLIMPORT_TEST(testTdf129888vml, "tdf129888vml.docx")
{
    //the line shape has anchor in the first cell however it has to
    //be positioned to an another cell. To reach this we must handle
    //the o:allowincell attribute of the shape, and its position has
    //to be calculated from the page frame instead of the table:

    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    sal_Int16 nValue;
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf129888vml The line shape has bad place!",
                                 text::RelOrientation::PAGE_FRAME, nValue);
}

DECLARE_OOXMLIMPORT_TEST(testTdf129888dml, "tdf129888dml.docx")
{
    //the shape has anchor in the first cell however it has to
    //be positioned to the right side of the page. To reach this we must handle
    //the layoutInCell attribute of the shape, and its position has
    //to be calculated from the page frame instead of the table:

    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    sal_Int16 nValue;
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf129888dml The shape has bad place!",
                                 text::RelOrientation::PAGE_FRAME, nValue);
}

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

DECLARE_OOXMLEXPORT_TEST(testTdf129522_removeShadowStyle, "tdf129522_removeShadowStyle.odt")
{
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference< beans::XPropertySet > xStyleProps(paragraphStyles->getByName("Shadow"), uno::UNO_QUERY_THROW);
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xStyleProps, "ParaShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);

    // Shadows were inherited regardless of whether the style disabled them.
    xStyleProps.set(paragraphStyles->getByName("Shadow-removed"), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, "ParaShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);

    uno::Reference< container::XNameAccess > characterStyles = getStyles("CharacterStyles");
    xStyleProps.set(characterStyles->getByName("CharShadow"), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, "CharShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);

    xStyleProps.set(characterStyles->getByName("CharShadow-removed"), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, "CharShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);
}

DECLARE_OOXMLEXPORT_TEST(testTdf130167_spilloverHeaderShape, "testTdf130167_spilloverHeader.docx")
{
    uno::Reference<text::XTextGraphicObjectsSupplier> xTextGraphicObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xNameAccess(
        xTextGraphicObjectsSupplier->getGraphicObjects(), uno::UNO_QUERY);
    // graphics from discarded headers were being added to the text body. Reduced from 5 to 2 shapes overall.
    CPPUNIT_ASSERT(xNameAccess->getCount() < 4);
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

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testTableStyleConfNested)
{
    // Create the doc model.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "table-style-conf-nested.docx";
    loadURL(aURL, nullptr);

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    validate(maTempFile.GetFileName(), test::OOXML);
    mbExported = true;
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed, as the custom table cell
    // border properties were lost, so the outer A2 cell started to have borders, not present in the
    // doc model.
    assertXPath(pXmlDoc, "//w:body/w:tbl/w:tr/w:tc[2]/w:tcPr/w:tcBorders/w:top", "val", "nil");
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testZeroLineSpacing)
{
    // Create the doc model.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    style::LineSpacing aSpacing;
    aSpacing.Mode = style::LineSpacingMode::MINIMUM;
    aSpacing.Height = 0;
    xParagraph->setPropertyValue("ParaLineSpacing", uno::makeAny(aSpacing));

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mbExported = true;
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: atLeast
    // - Actual  : auto
    // i.e. the minimal linespacing was lost on export.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:spacing", "lineRule", "atLeast");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:spacing", "line", "0");
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testSemiTransparentText)
{
    // Create an in-memory empty document.
    loadURL("private:factory/swriter", nullptr);

    // Set text to half-transparent and type a character.
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParagraph.is());
    sal_Int16 nTransparence = 75;
    xParagraph->setPropertyValue("CharTransparence", uno::makeAny(nTransparence));
    uno::Reference<text::XTextRange> xTextRange(xParagraph, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextRange.is());
    xTextRange->setString("x");

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mbExported = true;
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    OString aXPath
        = "/w:document/w:body/w:p/w:r/w:rPr/w14:textFill/w14:solidFill/w14:srgbClr/w14:alpha";
    double fValue = getXPath(pXmlDoc, aXPath, "val").toDouble();
    sal_Int16 nActual = basegfx::fround(fValue / oox::drawingml::PER_PERCENT);

    // Without the accompanying fix in place, this test would have failed, as the w14:textFill
    // element was missing.
    CPPUNIT_ASSERT_EQUAL(nTransparence, nActual);
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testUserField)
{
    // Create an in-memory empty document with a user field.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XDependentTextField> xField(
        xFactory->createInstance("com.sun.star.text.TextField.User"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMaster(
        xFactory->createInstance("com.sun.star.text.FieldMaster.User"), uno::UNO_QUERY);
    xMaster->setPropertyValue("Name", uno::makeAny(OUString("foo")));
    xField->attachTextFieldMaster(xMaster);
    xField->getTextFieldMaster()->setPropertyValue("Content", uno::makeAny(OUString("bar")));
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    xText->insertTextContent(xText->createTextCursor(), xField, /*bAbsorb=*/false);

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    validate(maTempFile.GetFileName(), test::OOXML);
    mbExported = true;
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed, the user field was
    // exported as <w:t>User Field foo = bar</w:t>.
    assertXPathContent(pXmlDoc, "//w:p/w:r[2]/w:instrText", " DOCVARIABLE foo ");
    assertXPathContent(pXmlDoc, "//w:p/w:r[4]/w:t", "bar");

    // Make sure that not only the variables, but also their values are written.
    pXmlDoc = parseExport("word/settings.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//w:docVars/w:docVar", "name", "foo");
    assertXPath(pXmlDoc, "//w:docVars/w:docVar", "val", "bar");
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

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf77796, "tdf77796.docx")
{
    xmlDocPtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    // cell paddings from table style
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:start", "w", "5");
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:top", "w", "240");
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:bottom", "w", "480");
    // not modified
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:end", "w", "108");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf128290, "tdf128290.odt")
{
    xmlDocPtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblLayout", "type", "fixed");
}

DECLARE_OOXMLEXPORT_TEST(testContSectBreakHeaderFooter, "cont-sect-break-header-footer.docx")
{
    // Load a document with a continuous section break on page 2.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header, section 1"),
                         parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer, section 1"),
                         parseDump("/root/page[1]/footer/txt/text()"));
    // Make sure the header stays like this; if we naively just update the page style name of the
    // first para on page 2, then this would be 'Header, section 2', which is incorrect.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header, section 2"),
                         parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer, section 2"),
                         parseDump("/root/page[2]/footer/txt/text()"));
    // This is inherited from page 2.
    CPPUNIT_ASSERT_EQUAL(OUString("Header, section 2"),
                         parseDump("/root/page[3]/header/txt/text()"));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - xpath should match exactly 1 node
    // i.e. the footer had no text (inherited from page 2), while the correct behavior is to provide
    // the own footer text.
    CPPUNIT_ASSERT_EQUAL(OUString("Footer, section 3"),
                         parseDump("/root/page[3]/footer/txt/text()"));

    // Without the export fix in place, the import-export-import test would have failed with:
    // - Expected: Header, section 2
    // - Actual  : First page header, section 2
    // i.e. both the header and the footer on page 3 was wrong.
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
