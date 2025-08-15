/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertyState.hpp>

#include <comphelper/configuration.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/propertyvalue.hxx>
#include <officecfg/Office/Common.hxx>

#include <pam.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <IDocumentSettingAccess.hxx>
#include <wrtsh.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr)
    {
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf166544_noTopMargin_fields, "tdf166544_noTopMargin_fields.docx")
{
    // given a document with a hyperlink field containing a page break
    auto pXmlDoc = parseLayoutDump();

    // The top margin is applied before the page break - since the page break follows the field end
    sal_Int32 nHeight = getXPath(pXmlDoc, "//page[2]//txt/infos/bounds", "height").toInt32();
    // Without the fix, the text height (showing a large top margin) was 569
    CPPUNIT_ASSERT_EQUAL(sal_Int32(269), nHeight);
}

DECLARE_OOXMLEXPORT_TEST(testTdf166510_sectPr_bottomSpacing, "tdf166510_sectPr_bottomSpacing.docx")
{
    // given with a sectPr with different bottom spacing (undefined in this case - i.e. zero)
    auto pXmlDoc = parseLayoutDump();

    // The last paragraph (sectPr) has 0 below spacing, so no reduction of page 2's 200pt top margin
    sal_Int32 nHeight = getXPath(pXmlDoc, "//page[2]//body/txt/infos/bounds", "height").toInt32();
    // Without the fix, the text height (showing no top margin at all) was 253
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4253), nHeight);
}

DECLARE_OOXMLEXPORT_TEST(testTdf167657_sectPr_bottomSpacing, "tdf167657_sectPr_bottomSpacing.docx")
{
    // given with a continuous break sectPr with no belowSpacing
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    auto pXmlDoc = parseLayoutDump();

    // Since there is no page break, the prior paragraph's belowSpacing should not be zero'd out.
    // NOTE: apparently layout assigns the belowSpacing to the following section, not body/txt[1],
    sal_Int32 nHeight = getXPath(pXmlDoc, "//section/infos/bounds", "height").toInt32();
    // Without the fix, the section's height (showing no bottom margin for the 1st para) was 309
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1409), nHeight);
}

DECLARE_OOXMLEXPORT_TEST(testTdf165478_bottomAligned, "tdf165478_bottomAligned.docx")
{
    // given a layoutInCell, wrap-through image, paragraph-anchored to a bottom-aligned cell
    auto pXmlDoc = parseLayoutDump();

    // The text in the cell should be at the bottom of the cell
    assertXPathContent(pXmlDoc, "//cell[2]/txt", u"Bottom aligned");
    sal_Int32 nTextBottom = getXPath(pXmlDoc, "//cell[2]/txt/infos/bounds", "bottom").toInt32();
    sal_Int32 nCellBottom = getXPath(pXmlDoc, "//cell[2]/infos/bounds", "bottom").toInt32();

    // Without the fix, the text was at the top of the cell (2002) instead of at the bottom (4423)
    CPPUNIT_ASSERT_EQUAL(nCellBottom, nTextBottom);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf166620)
{
    createSwDoc();
    {
        SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
        pWrtShell->Insert(u"Body text"_ustr);
        pWrtShell->InsertFootnote({}, /*bEndNote=*/true, /*bEdit=*/true);
        pWrtShell->Insert(u"Endnote text"_ustr);
    }

    // Exporting to a Word format, a tab is prepended to the endnote text. When imported, the
    // NoGapAfterNoteNumber compatibility flag is enabled; and the exported tab is the only thing
    // that separates the number and the text. The tab must not be stripped away on import.
    saveAndReload(mpFilter);
    {
        auto xFactory = mxComponent.queryThrow<lang::XMultiServiceFactory>();
        auto xSettings = xFactory->createInstance(u"com.sun.star.document.Settings"_ustr);
        CPPUNIT_ASSERT(getProperty<bool>(xSettings, u"NoGapAfterNoteNumber"_ustr));

        auto xSupplier = mxComponent.queryThrow<text::XEndnotesSupplier>();
        auto xEndnotes = xSupplier->getEndnotes();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xEndnotes->getCount());
        auto xEndnoteText = xEndnotes->getByIndex(0).queryThrow<text::XText>();
        CPPUNIT_ASSERT_EQUAL(u"\tEndnote text"_ustr, xEndnoteText->getString());
    }
    // Do a second round-trip. It must not duplicate the tab.
    saveAndReload(mpFilter);
    {
        auto xFactory = mxComponent.queryThrow<lang::XMultiServiceFactory>();
        auto xSettings = xFactory->createInstance(u"com.sun.star.document.Settings"_ustr);
        CPPUNIT_ASSERT(getProperty<bool>(xSettings, u"NoGapAfterNoteNumber"_ustr));

        auto xSupplier = mxComponent.queryThrow<text::XEndnotesSupplier>();
        auto xEndnotes = xSupplier->getEndnotes();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xEndnotes->getCount());
        auto xEndnoteText = xEndnotes->getByIndex(0).queryThrow<text::XText>();
        CPPUNIT_ASSERT_EQUAL(u"\tEndnote text"_ustr, xEndnoteText->getString());

        // Remove the tab
        xEndnoteText->setString(u"Endnote text"_ustr);
    }
    // Do a third round-trip. It must not introduce the tab, because of the compatibility flag.
    saveAndReload(mpFilter);
    {
        auto xFactory = mxComponent.queryThrow<lang::XMultiServiceFactory>();
        auto xSettings = xFactory->createInstance(u"com.sun.star.document.Settings"_ustr);
        CPPUNIT_ASSERT(getProperty<bool>(xSettings, u"NoGapAfterNoteNumber"_ustr));

        auto xSupplier = mxComponent.queryThrow<text::XEndnotesSupplier>();
        auto xEndnotes = xSupplier->getEndnotes();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xEndnotes->getCount());
        auto xEndnoteText = xEndnotes->getByIndex(0).queryThrow<text::XText>();
        CPPUNIT_ASSERT_EQUAL(u"Endnote text"_ustr, xEndnoteText->getString());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf167082)
{
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Heading 1
    // - Actual  : Standard

    createSwDoc("tdf167082.docx");
    saveAndReload(mpFilter);
    OUString aStyleName = getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"), aStyleName);
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTableAnchorPosExport)
{
    // Given a document with two floating tables after each other:
    // When saving that document to DOCX:
    loadAndSave("floattable-anchorpos.docx");

    // Then make sure that the dummy anchor of the first floating table is not written to the export
    // result:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Check the order of the floating tables: C is from the previous node, A is normal floating
    // table.
    CPPUNIT_ASSERT_EQUAL(u"C"_ustr,
                         getXPathContent(pXmlDoc, "//w:body/w:tbl[1]/w:tr/w:tc/w:p/w:r/w:t"));
    CPPUNIT_ASSERT_EQUAL(u"A"_ustr,
                         getXPathContent(pXmlDoc, "//w:body/w:tbl[2]/w:tr/w:tc/w:p/w:r/w:t"));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the dummy anchor node was written to DOCX, leading to a Writer vs Word layout
    // difference.
    CPPUNIT_ASSERT_EQUAL(1, countXPathNodes(pXmlDoc, "//w:body/w:p"));
    CPPUNIT_ASSERT_EQUAL(u"D"_ustr, getXPathContent(pXmlDoc, "//w:body/w:p/w:r/w:t"));
}

CPPUNIT_TEST_FIXTURE(Test, testFontEmbeddingDOCX)
{
    createSwDoc("font_used_in_header_only.fodt");
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xProps(
        xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(uno::Any(true), xProps->getPropertyValue(u"EmbedFonts"_ustr));

    save(mpFilter);

    xmlDocUniquePtr pXml = parseExport(u"word/fontTable.xml"_ustr);

    // Test that DejaVu Sans is embedded
    assertXPath(pXml, "/w:fonts/w:font[@w:name='DejaVu Sans']/w:embedRegular");
    assertXPath(pXml, "/w:fonts/w:font[@w:name='DejaVu Sans']/w:embedBold");
    assertXPath(pXml, "/w:fonts/w:font[@w:name='DejaVu Sans']/w:embedItalic");
// It is strange that DejaVu is different on Linux: see e.g. tdf166627 in odfexport2.cxx
#if defined(_WIN32) || defined(MACOSX)
    assertXPath(pXml, "/w:fonts/w:font[@w:name='DejaVu Sans']/w:embedBoldItalic");
#endif

    // Test that common fonts (here: Liberation Serif, Liberation Sans) are not embedded
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Serif']");
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Serif']/w:embedRegular", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Serif']/w:embedBold", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Serif']/w:embedItalic", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Serif']/w:embedBoldItalic", 0);

    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Sans']");
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Sans']/w:embedRegular", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Sans']/w:embedBold", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Sans']/w:embedItalic", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Sans']/w:embedBoldItalic", 0);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
