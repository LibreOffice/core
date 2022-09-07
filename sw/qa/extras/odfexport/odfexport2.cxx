/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/text/ColumnSeparatorStyle.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <unotools/localedatawrapper.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/odfexport/data/", "writer8") {}

    bool mustValidate(const char* /*filename*/) const override
    {
        return true;
    }

};

DECLARE_ODFEXPORT_TEST(testTdf52065_centerTabs, "testTdf52065_centerTabs.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    sal_Int32 nTabStop = parseDump("//body/txt[4]/SwParaPortion/SwLineLayout/child::*[3]", "width").toInt32();
    // Without the fix, the text was unseen, with a tabstop width of 64057. It should be 3057
    CPPUNIT_ASSERT(nTabStop < 4000);
    CPPUNIT_ASSERT(3000 < nTabStop);
    CPPUNIT_ASSERT_EQUAL(OUString(u"Pečiatka zamestnávateľa"), parseDump("//body/txt[4]/SwParaPortion/SwLineLayout/child::*[4]", "portion"));

    // tdf#149547: __XXX___invalid CharacterStyles should not be imported/exported
    CPPUNIT_ASSERT(!getStyles("CharacterStyles")->hasByName("__XXX___invalid"));
}

DECLARE_ODFEXPORT_TEST(testTdf104254_noHeaderWrapping, "tdf104254_noHeaderWrapping.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    sal_Int32 nParaHeight = getXPath(pXmlDoc, "//header/txt[1]/infos/bounds", "height").toInt32();
    // The wrapping on header images is supposed to be ignored (since OOo for MS compat reasons),
    // thus making the text run underneath the image. Before, height was 1104. Now it is 552.
    CPPUNIT_ASSERT_MESSAGE("Paragraph should fit on a single line", nParaHeight < 600);
}

DECLARE_ODFEXPORT_TEST(testTdf131025_noZerosInTable, "tdf131025_noZerosInTable.odt")
{
    uno::Reference<text::XTextTablesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTables = xSupplier->getTextTables();
    uno::Reference<text::XTextTable> xTable(xTables->getByName("Table1"), uno::UNO_QUERY);

    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("C3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("5 gp"), xCell->getString());
}

DECLARE_ODFEXPORT_TEST(testTdf143793_noBodyWrapping, "tdf143793_noBodyWrapping.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    // Preserve old document wrapping. Compat "Use OOo 1.1 text wrapping around objects"
    // Originally, the body text did not wrap around spill-over header images
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Fits on one page", 1, getPages() );

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    sal_Int32 nParaHeight = getXPath(pXmlDoc, "//page[1]/header/txt[1]/infos/bounds", "height").toInt32();
    // The header text should wrap around the header image in OOo 1.1 and prior,
    // thus taking up two lines instead of one. One line is 276. It should be 552.
    CPPUNIT_ASSERT_MESSAGE("Header text should fill two lines", nParaHeight > 400);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf137199)
{
    loadAndReload("tdf137199.docx");
    CPPUNIT_ASSERT_EQUAL(OUString(">1<"), getProperty<OUString>(getParagraph(1), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("1)"), getProperty<OUString>(getParagraph(2), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("HELLO1WORLD!"), getProperty<OUString>(getParagraph(3), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("HELLO2WORLD!"), getProperty<OUString>(getParagraph(4), "ListLabelString"));
}

DECLARE_ODFEXPORT_TEST(testTdf143605, "tdf143605.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // With numbering type "none" there should be just prefix & suffix
    CPPUNIT_ASSERT_EQUAL(OUString("."), getProperty<OUString>(getParagraph(1), "ListLabelString"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf57317_autoListName)
{
    load(mpTestDocumentPath, "tdf57317_autoListName.odt");
    // The list style (from styles.xml) overrides a duplicate named auto-style
    //uno::Any aNumStyle = getStyles("NumberingStyles")->getByName("L1");
    //CPPUNIT_ASSERT(aNumStyle.hasValue());
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(">1<"), getProperty<OUString>(xPara, "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString("L1"), getProperty<OUString>(xPara, "NumberingStyleName"));

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:DefaultBullet", {});

    // This was failing with a duplicate auto numbering style name of L1 instead of a unique name,
    // thus it was showing the same info as before the bullet modification.
    reload(mpFilter, "");
    xPara.set(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara, "ListLabelString"));

    uno::Reference<container::XIndexAccess> xLevels(xPara->getPropertyValue("NumberingRules"),
                                                    uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps;
    for (beans::PropertyValue const& rProp : std::as_const(aProps))
    {
        if (rProp.Name == "BulletChar")
            return;
    }
    CPPUNIT_FAIL("no BulletChar property");
}

CPPUNIT_TEST_FIXTURE(Test, testListFormatDocx)
{
    loadAndReload("listformat.docx");
    // Ensure in resulting ODT we also have not just prefix/suffix, but custom delimiters
    CPPUNIT_ASSERT_EQUAL(OUString(">1<"), getProperty<OUString>(getParagraph(1), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString(">>1/1<<"), getProperty<OUString>(getParagraph(2), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString(">>1/1/1<<"), getProperty<OUString>(getParagraph(3), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString(">>1/1/2<<"), getProperty<OUString>(getParagraph(4), "ListLabelString"));

    // Check also that in numbering styles we have num-list-format defined
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='1']", "num-list-format", ">%1%<");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='2']", "num-list-format", ">>%1%/%2%<<");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='3']", "num-list-format", ">>%1%/%2%/%3%<<");

    // But for compatibility there are still prefix/suffix
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='1']", "num-prefix", ">");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='1']", "num-suffix", "<");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='2']", "num-prefix", ">>");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='2']", "num-suffix", "<<");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='3']", "num-prefix", ">>");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='3']", "num-suffix", "<<");
}

DECLARE_ODFEXPORT_TEST(testShapeWithHyperlink, "shape-with-hyperlink.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (xmlDocUniquePtr pXmlDoc = parseExport("content.xml"))
    {
        // Check how conversion from prefix/suffix to list format did work
        assertXPath(pXmlDoc, "/office:document-content/office:body/office:text/text:p/draw:a",
                    "href", "http://shape.com/");
    }
}

DECLARE_ODFEXPORT_TEST(testShapesHyperlink, "shapes-hyperlink.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(5, getShapes());
    uno::Reference<beans::XPropertySet> const xPropSet1(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("http://libreoffice.org/"), getProperty<OUString>(xPropSet1, "Hyperlink"));

    uno::Reference<beans::XPropertySet> const xPropSet2(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("http://libreoffice2.org/"), getProperty<OUString>(xPropSet2, "Hyperlink"));

    uno::Reference<beans::XPropertySet> const xPropSet3(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("http://libreoffice3.org/"), getProperty<OUString>(xPropSet3, "Hyperlink"));

    uno::Reference<beans::XPropertySet> const xPropSet4(getShape(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("http://libreoffice4.org/"), getProperty<OUString>(xPropSet4, "Hyperlink"));

    uno::Reference<beans::XPropertySet> const xPropSet5(getShape(5), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("http://libreoffice5.org/"), getProperty<OUString>(xPropSet5, "Hyperlink"));
}

DECLARE_ODFEXPORT_TEST(testListFormatOdt, "listformat.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Ensure in resulting ODT we also have not just prefix/suffix, but custom delimiters
    CPPUNIT_ASSERT_EQUAL(OUString(">1<"), getProperty<OUString>(getParagraph(1), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString(">>1.1<<"), getProperty<OUString>(getParagraph(2), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString(">>1.1.1<<"), getProperty<OUString>(getParagraph(3), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString(">>1.1.2<<"), getProperty<OUString>(getParagraph(4), "ListLabelString"));

    if (xmlDocUniquePtr pXmlDoc = parseExport("content.xml"))
    {
        // Check how conversion from prefix/suffix to list format did work
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='1']", "num-list-format", ">%1%<");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='2']", "num-list-format", ">>%1%.%2%<<");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='3']", "num-list-format", ">>%1%.%2%.%3%<<");

        // But for compatibility there are still prefix/suffix as they were before
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='1']", "num-prefix", ">");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='1']", "num-suffix", "<");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='2']", "num-prefix", ">>");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='2']", "num-suffix", "<<");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='3']", "num-prefix", ">>");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='3']", "num-suffix", "<<");
    }
}

CPPUNIT_TEST_FIXTURE(Test, testStyleLink)
{
    // Given a document with a para and a char style that links each other, when loading that
    // document:
    load(mpTestDocumentPath, "style-link.fodt");

    // Then make sure the char style links the para one:
    uno::Any aCharStyle = getStyles("CharacterStyles")->getByName("List Paragraph Char");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: List Paragraph
    // - Actual  :
    // i.e. the linked style was lost on import.
    CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"), getProperty<OUString>(aCharStyle, "LinkStyle"));
    uno::Any aParaStyle = getStyles("ParagraphStyles")->getByName("List Paragraph");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: List Paragraph Char
    // - Actual  :
    // i.e. the linked style was lost on import.
    CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph Char"), getProperty<OUString>(aParaStyle, "LinkStyle"));
}

CPPUNIT_TEST_FIXTURE(Test, tdf120972)
{
    loadAndReload("table_number_format_3.docx");

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    OUString cDecimal(SvtSysLocale().GetLocaleData().getNumDecimalSep()[0]);
    assertXPath(
        pXmlDoc,
        "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/style:tab-stop",
        "char", cDecimal);
    assertXPath(
        pXmlDoc,
        "//style:style[@style:name='P2']/style:paragraph-properties/style:tab-stops/style:tab-stop",
        "char", cDecimal);
}

DECLARE_ODFEXPORT_TEST(testSectionColumnSeparator, "section-columns-separator.fodt")
{
    // tdf#150235: due to wrong types used in column export, 'style:height' and 'style:style'
    // attributes were exported incorrectly for 'style:column-sep' element
    auto xSection = getProperty<uno::Reference<uno::XInterface>>(getParagraph(1), "TextSection");
    auto xColumns = getProperty<uno::Reference<text::XTextColumns>>(xSection, "TextColumns");
    CPPUNIT_ASSERT(xColumns);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xColumns->getColumnCount());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 50
    // - Actual  : 100
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(50),
                         getProperty<sal_Int32>(xColumns, "SeparatorLineRelativeHeight"));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(css::text::ColumnSeparatorStyle::DOTTED,
                         getProperty<sal_Int16>(xColumns, "SeparatorLineStyle"));

    // Check the rest of the properties, too
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xColumns, "IsAutomatic"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(600), getProperty<sal_Int32>(xColumns, "AutomaticDistance"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), getProperty<sal_Int32>(xColumns, "SeparatorLineWidth"));
    CPPUNIT_ASSERT_EQUAL(Color(0x99, 0xAA, 0xBB),
                         getProperty<Color>(xColumns, "SeparatorLineColor"));
    CPPUNIT_ASSERT_EQUAL(
        css::style::VerticalAlignment_BOTTOM,
        getProperty<css::style::VerticalAlignment>(xColumns, "SeparatorLineVerticalAlignment"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xColumns, "SeparatorLineIsOn"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
