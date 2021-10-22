/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <unotxdoc.hxx>

#include <com/sun/star/text/XTextTable.hpp>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/odfexport/data/", "writer8") {}

    /**
     * Denylist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // Only test import of .odt document
        return OString(filename).endsWith(".odt");
    }

    bool mustValidate(const char* /*filename*/) const override
    {
        return true;
    }

};

DECLARE_ODFEXPORT_TEST(testTdf52065_centerTabs, "testTdf52065_centerTabs.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    sal_Int32 nTabStop = parseDump("//body/txt[4]/Text[3]", "nWidth").toInt32();
    // Without the fix, the text was unseen, with a tabstop width of 64057. It should be 3057
    CPPUNIT_ASSERT(nTabStop < 4000);
    CPPUNIT_ASSERT(3000 < nTabStop);
    CPPUNIT_ASSERT_EQUAL(OUString(u"Pečiatka zamestnávateľa"), parseDump("//body/txt[4]/Text[4]", "Portion"));
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

DECLARE_ODFEXPORT_TEST(testTdf137199, "tdf137199.docx")
{
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

DECLARE_ODFEXPORT_TEST(testListFormatDocx, "listformat.docx")
{
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

// This test started in LO 7.2. Use the odfexport.cxx if you intend to backport to 7.1.

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
