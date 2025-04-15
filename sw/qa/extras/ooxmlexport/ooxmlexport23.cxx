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
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <editeng/unolingu.hxx>

#include <comphelper/sequenceashashmap.hxx>
#include <officecfg/Office/Common.hxx>

using namespace com::sun::star;

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testHighlightEdit_numbering)
{
    // Create the doc model.
    createSwDoc("tdf135774_numberingCRProps.docx");

    // This only affects when saving as w:highlight - which is not the default since 7.0.
    bool bWasExportToShade = !officecfg::Office::Common::Filter::Microsoft::Export::
                                 CharBackgroundToHighlighting::get();
    auto batch = comphelper::ConfigurationChanges::create();
    officecfg::Office::Common::Filter::Microsoft::Export::CharBackgroundToHighlighting::set(true,
                                                                                            batch);
    batch->commit();

    //Simulate a user editing the char background color of the paragraph 2 marker (CR)
    uno::Reference<beans::XPropertySet> properties(getParagraph(2), uno::UNO_QUERY);
    uno::Sequence<beans::NamedValue> aListAutoFormat;
    CPPUNIT_ASSERT(properties->getPropertyValue(u"ListAutoFormat"_ustr) >>= aListAutoFormat);
    comphelper::SequenceAsHashMap aMap(properties->getPropertyValue(u"ListAutoFormat"_ustr));
    // change the background color to RES_CHRATR_BACKGROUND.
    aMap[u"CharBackColor"_ustr] <<= static_cast<sal_Int32>(0xff00ff);
    // Two attributes can affect character background. Highlight has priority, and is only there for MS compatibility,
    // so clear any potential highlight set earlier, or override any coming via a style.
    aMap[u"CharHighlight"_ustr] <<= static_cast<sal_Int32>(COL_TRANSPARENT);

    uno::Sequence<beans::PropertyValue> aGrabBag;
    aMap[u"CharInteropGrabBag"_ustr] >>= aGrabBag;
    for (beans::PropertyValue& rProp : asNonConstRange(aGrabBag))
    {
        // The shading is no longer defined from import, so clear that flag.
        // BackColor 0xff00ff will now attempt to export as highlight, since we set that in officecfg.
        if (rProp.Name == "CharShadingMarker")
            rProp.Value <<= false;
    }
    aMap[u"CharInteropGrabBag"_ustr] <<= aGrabBag;

    aMap >> aListAutoFormat;
    properties->setPropertyValue(u"ListAutoFormat"_ustr, uno::Any(aListAutoFormat));

    // Export to docx.
    save(u"Office Open XML Text"_ustr);

    // Paragraph 2 should have only one w:highlight written per w:rPr. Without the fix, there were two.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//w:body/w:p[2]/w:pPr/w:rPr/w:highlight", "val", u"none");
    // Visually, the "none" highlight means the bullet point should not have a character background.

    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);
    assertXPath(pXmlStyles,
                "//w:style[@w:styleId='CustomParaStyleHighlightGreen']/w:rPr/w:highlight", "val",
                u"green");
    // Visually, the last bullet point's text should be green-highlighted (but the bullet point itself shouldn't)

    if (bWasExportToShade)
    {
        officecfg::Office::Common::Filter::Microsoft::Export::CharBackgroundToHighlighting::set(
            false, batch);
        batch->commit();
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132766)
{
    loadAndSave("tdf132766.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // Ensure that for list=1 and level=0 we wrote correct bullet char and correct font
    assertXPath(pXmlDoc,
                "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:lvlText",
                "val", u"\uF0B7");
    assertXPath(
        pXmlDoc,
        "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:rFonts",
        "ascii", u"Symbol");
    assertXPath(
        pXmlDoc,
        "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:rFonts",
        "hAnsi", u"Symbol");
    assertXPath(
        pXmlDoc,
        "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:rFonts", "cs",
        u"Symbol");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128245)
{
    loadAndSave("tdf128245.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "//w:num[@w:numId='1']/w:abstractNumId", "val", u"1");
    assertXPath(pXmlDoc, "//w:num[@w:numId='2']/w:abstractNumId", "val", u"2");
    assertXPath(pXmlDoc, "//w:num[@w:numId='3']/w:abstractNumId", "val", u"3");
    assertXPath(pXmlDoc, "//w:num[@w:numId='4']/w:abstractNumId", "val", u"1");
    assertXPath(pXmlDoc, "//w:num[@w:numId='4']/w:lvlOverride[@w:ilvl='0']", "ilvl", u"0");
    assertXPath(pXmlDoc, "//w:num[@w:numId='4']/w:lvlOverride/w:startOverride", "val", u"1");
    assertXPath(pXmlDoc, "//w:num[@w:numId='4']/w:lvlOverride[@w:ilvl='1']", 0);
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
                             xTableRows->getByIndex(2), u"TableColumnSeparators"_ustr)[0]
                             .Position);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf95189)
{
    loadAndReload("tdf95189.docx");
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"2"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"2"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"3"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(6), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(7), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"2"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128820)
{
    loadAndSave("tdf128820.fodt");
    // Import of exported DOCX failed because of wrong namespace used for wsp element
    // Now test the exported XML, in case we stop failing opening invalid files
    xmlDocUniquePtr pXml = parseExport(u"word/document.xml"_ustr);
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

CPPUNIT_TEST_FIXTURE(Test, testTdf128889)
{
    loadAndSave("tdf128889.fodt");
    xmlDocUniquePtr pXml = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXml);
    // There was an w:r (with w:br) as an invalid child of first paragraph's w:pPr
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:pPr/w:r", 0);
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:r", 2);
    // Check that the break is in proper - last - position
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:r[2]/w:br", "type", u"page");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132754)
{
    loadAndReload("tdf132754.docx");
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"0.0.0."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"0.0.1."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"0.0.2."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf129353, "tdf129353.docx")
{
    CPPUNIT_ASSERT_EQUAL(8, getParagraphs());
    getParagraph(1, u"(Verne, 1870)"_ustr);
    getParagraph(2, u"Bibliography"_ustr);
    getParagraph(4, u"Christie, A. (1922). The Secret Adversary. "_ustr);
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
    CPPUNIT_ASSERT_EQUAL(u"\n" // starting with an empty paragraph
                         "Christie, A. (1922). The Secret Adversary. \n"
                         "\n"
                         "Verne, J. G. (1870). Twenty Thousand Leagues Under the Sea. \n"
                         ""_ustr, // ending with an empty paragraph
                         aIndexString);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf77796)
{
    loadAndSave("tdf77796.docx");
    xmlDocUniquePtr pXml = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXml);
    // cell paddings from table style
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:start", "w", u"5");
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:top", "w", u"240");
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:bottom", "w", u"480");
    // not modified
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:end", "w", u"108");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128290)
{
    loadAndSave("tdf128290.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXml = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblLayout", "type", u"fixed");

    // ensure unnecessary suppressLineNumbers entry is not created.
    xmlDocUniquePtr pStylesXml = parseExport(u"word/styles.xml"_ustr);
    assertXPath(pStylesXml, "//w:style[@w:styleId='Normal']/w:pPr/w:suppressLineNumbers", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf120394, "tdf120394.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"1.2.1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf133605, "tdf133605.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"1."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"1.1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(6), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(3),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"(a)"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf133605_2, "tdf133605_2.docx")
{
    // About the same document as tdf133605.docx, but number definition has level definitions in random order
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"1."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"1.1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(6), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(3),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"(a)"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123757)
{
    loadAndSave("tdf123757.docx");
    xmlDocUniquePtr pXml = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "/w:document/w:body/w:tbl", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf141172)
{
    loadAndSave("tdf141172.docx");
    xmlDocUniquePtr pXml = parseExport(u"word/endnotes.xml"_ustr);
    CPPUNIT_ASSERT(pXml);
    // This was 1 (lost table during copying endnote content)
    assertXPath(pXml, "/w:endnotes/w:endnote/w:tbl", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf141548)
{
    loadAndSave("tdf141548.docx");
    xmlDocUniquePtr pXml = parseExport(u"word/endnotes.xml"_ustr);
    CPPUNIT_ASSERT(pXml);
    // This was 0 (lost text content of the run with endnoteRef)
    assertXPath(pXml, "/w:endnotes/w:endnote[4]/w:p/w:r[2]/w:t", 2);
    assertXPathContent(pXml, "/w:endnotes/w:endnote[4]/w:p/w:r[2]/w:t[1]", u"another endnote");
    assertXPathContent(pXml, "/w:endnotes/w:endnote[4]/w:p/w:r[2]/w:t[2]", u"new line");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143399)
{
    loadAndSave("tdf143399.docx");
    xmlDocUniquePtr pXml = parseExport(u"word/footnotes.xml"_ustr);
    CPPUNIT_ASSERT(pXml);
    // These were 0 (lost text content of documents both with footnotes and endnotes)
    assertXPath(pXml, "/w:footnotes/w:footnote[3]/w:p/w:r[3]/w:t", 1);
    assertXPathContent(pXml, "/w:footnotes/w:footnote[3]/w:p/w:r[3]/w:t", u"Footnotes_graphic2");
    assertXPath(pXml, "/w:footnotes/w:footnote[4]/w:p/w:r[3]/w:t", 1);
    assertXPathContent(pXml, "/w:footnotes/w:footnote[4]/w:p/w:r[3]/w:t", u"Footnotes_graphic");

    xmlDocUniquePtr pXml2 = parseExport(u"word/endnotes.xml"_ustr);
    CPPUNIT_ASSERT(pXml);
    // This was 0 (lost text content of the run with endnoteRef)
    assertXPath(pXml2, "/w:endnotes/w:endnote[3]/w:p/w:r[3]/w:t", 1);
    assertXPathContent(pXml2, "/w:endnotes/w:endnote[3]/w:p/w:r[3]/w:t[1]", u"Endnotes");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143583)
{
    loadAndSave("tdf143583_emptyParaAtEndOfFootnote.docx");
    xmlDocUniquePtr pXml = parseExport(u"word/footnotes.xml"_ustr);
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "/w:footnotes/w:footnote[3]/w:p", 2);
    // This was 1
    assertXPath(pXml, "/w:footnotes/w:footnote[4]/w:p", 2);
    // This was 2
    assertXPath(pXml, "/w:footnotes/w:footnote[5]/w:p", 3);
    // This was 2
    assertXPath(pXml, "/w:footnotes/w:footnote[6]/w:p", 3);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152203)
{
    loadAndSave("tdf152203.docx");
    xmlDocUniquePtr pXml = parseExport(u"word/footnotes.xml"_ustr);
    CPPUNIT_ASSERT(pXml);

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xLastFootnote(xFootnotes->getByIndex(5), uno::UNO_QUERY);
    // This was "Footnote for pg5" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL(u"Footnote for pg 6"_ustr, xLastFootnote->getString().trim());

    uno::Reference<text::XTextRange> xLastButOne(xFootnotes->getByIndex(4), uno::UNO_QUERY);
    // This was "Footnote for pg 6" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL(u"Footnote for pg5"_ustr, xLastButOne->getString().trim());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152506)
{
    loadAndSave("tdf152506.docx");
    xmlDocUniquePtr pXml = parseExport(u"word/footnotes.xml"_ustr);
    CPPUNIT_ASSERT(pXml);

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xLastFootnote(xFootnotes->getByIndex(1), uno::UNO_QUERY);
    // This was "Footnote for pg5" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL(u"Footnote for pg 6"_ustr, xLastFootnote->getString().trim());

    uno::Reference<text::XTextRange> xLastButOne(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This was "Footnote for pg 6" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL(u"Footnote for pg5"_ustr, xLastButOne->getString().trim());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153255)
{
    loadAndSave("tdf153255.docx");
    xmlDocUniquePtr pXml = parseExport(u"word/footnotes.xml"_ustr);
    CPPUNIT_ASSERT(pXml);

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xLastFootnote(xFootnotes->getByIndex(5), uno::UNO_QUERY);
    // This was "Footnote for pg2" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL(u"Footnote for pg 6"_ustr, xLastFootnote->getString().trim());

    uno::Reference<text::XTextRange> xLastButOne(xFootnotes->getByIndex(4), uno::UNO_QUERY);
    // This was "Footnote for pg 6" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL(u"Footnote for pg5"_ustr, xLastButOne->getString().trim());

    // check all the remaining footnotes

    uno::Reference<text::XTextRange> xFootnote1(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This was "Footnote for pg3" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL(u"Footnote for pg1"_ustr, xFootnote1->getString().trim());

    uno::Reference<text::XTextRange> xFootnote2(xFootnotes->getByIndex(1), uno::UNO_QUERY);
    // This was "Footnote for pg5" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL(u"Footnote for pg2"_ustr, xFootnote2->getString().trim());

    uno::Reference<text::XTextRange> xFootnote3(xFootnotes->getByIndex(2), uno::UNO_QUERY);
    // This was "Footnote for pg4." (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL(u"Footnote for pg3"_ustr, xFootnote3->getString().trim());

    uno::Reference<text::XTextRange> xFootnote4(xFootnotes->getByIndex(3), uno::UNO_QUERY);
    // This was "Footnote for pg1" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL(u"Footnote for pg4."_ustr, xFootnote4->getString().trim());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153804)
{
    loadAndSave("tdf153804.docx");
    xmlDocUniquePtr pXml = parseExport(u"word/footnotes.xml"_ustr);
    CPPUNIT_ASSERT(pXml);

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xLastFootnote(xFootnotes->getByIndex(1), uno::UNO_QUERY);
    // This was empty
    CPPUNIT_ASSERT_EQUAL(u"Footnote for pg 6"_ustr, xLastFootnote->getString().trim());

    uno::Reference<text::XTextRange> xLastButOne(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This was empty
    CPPUNIT_ASSERT_EQUAL(u"Footnote for pg5"_ustr, xLastButOne->getString().trim());
}

// skip test for macOS (missing fonts?)
#if !defined(MACOSX)
DECLARE_OOXMLEXPORT_TEST(testTdf146346, "tdf146346.docx")
{
    // This was 2 (by bad docDefault vertical margins around tables in footnotes)
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // only first page has table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // check first page: all tables on the first page
    assertXPath(pXmlDoc, "/root/page[1]//anchored/fly", 8);
    assertXPath(pXmlDoc, "/root/page[1]//anchored/fly/tab", 8);

    // No second page.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. unwanted lower margin in the floating table's anchor paragraph in the footnote created a
    // second page.
    assertXPath(pXmlDoc, "/root/page[2]", 0);
}
#endif

DECLARE_OOXMLEXPORT_TEST(testTdf130088, "tdf130088.docx")
{
    // This was 2 (justification without shrinking resulted more lines)
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // check compatibility option in ODT export/import, too
    saveAndReload(u"writer8"_ustr);

    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf158436, "tdf158436.docx")
{
    // This resulted freezing
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf159032, "tdf159032.docx")
{
    // This resulted crashing
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf160518, "tdf160518_useWord2013TrackBottomHyphenation.docx")
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // This was 2 (without shifting last hyphenated line of the page)
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf160518_compatible, "tdf160518_allowHyphenationAtTrackBottom.docx")
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // This is still 2
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf160518_ODT, "tdf160518_useWord2013TrackBottomHyphenation.docx")
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // This was 2 (without shifting last hyphenated line of the page)
    CPPUNIT_ASSERT_EQUAL(3, getPages());

    // check compatibility option in ODT export/import, too
    saveAndReload(u"writer8"_ustr);

    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf160518_ODT_compatible,
                         "tdf160518_allowHyphenationAtTrackBottom.docx")
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // This is still 2
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // check compatibility option in ODT export/import, too
    saveAndReload(u"writer8"_ustr);

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160518_page_in_default_paragraph_style)
{
    // default paragraph style contains hyphenation settings
    loadAndReload("tdf160518_page_in_default_paragraph_style.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlDoc,
                "/w:settings/w:compat/w:compatSetting[@w:name='useWord2013TrackBottomHyphenation']",
                "val", u"1");
    assertXPath(pXmlDoc,
                "/w:settings/w:compat/w:compatSetting[@w:name='allowHyphenationAtTrackBottom']", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160518_auto_in_default_paragraph_style)
{
    // default paragraph style contains hyphenation settings
    loadAndReload("tdf160518_auto_in_default_paragraph_style.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlDoc,
                "/w:settings/w:compat/w:compatSetting[@w:name='useWord2013TrackBottomHyphenation']",
                "val", u"1");
    assertXPath(pXmlDoc,
                "/w:settings/w:compat/w:compatSetting[@w:name='allowHyphenationAtTrackBottom']",
                "val", u"1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160518_page_in_text_body_style)
{
    // text body style contains hyphenation settings
    loadAndReload("tdf160518_page_in_text_body_style.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlDoc,
                "/w:settings/w:compat/w:compatSetting[@w:name='useWord2013TrackBottomHyphenation']",
                "val", u"1");
    assertXPath(pXmlDoc,
                "/w:settings/w:compat/w:compatSetting[@w:name='allowHyphenationAtTrackBottom']", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160518_auto_in_text_body_style)
{
    // text body style contains hyphenation settings
    loadAndReload("tdf160518_auto_in_text_body_style.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlDoc,
                "/w:settings/w:compat/w:compatSetting[@w:name='useWord2013TrackBottomHyphenation']",
                "val", u"1");
    assertXPath(pXmlDoc,
                "/w:settings/w:compat/w:compatSetting[@w:name='allowHyphenationAtTrackBottom']",
                "val", u"1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165354)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    loadAndReload("tdf165354.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was "except that it has an at" (hyphenation at the end of the page)
    assertXPath(pXmlDoc, "//page[1]/body/txt[2]/SwParaPortion/SwLineLayout[9]", "portion",
                u"except that it has an ");
    // This started with "mosphere" (hyphenation at the end of the previous page)
    assertXPath(pXmlDoc, "//page[2]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u"atmosphere. The Earth ");
    // The same word is still hyphenated in the same paragraph, but not at the bottom of the page
    assertXPath(pXmlDoc, "//page[2]/body/txt[1]/SwParaPortion/SwLineLayout[9]", "portion",
                u"except that it has an at");
}

CPPUNIT_TEST_FIXTURE(Test, testHyphenationAuto)
{
    loadAndReload("hyphenation.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Explicitly set hyphenation=auto on document level
    xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
    CPPUNIT_ASSERT(pXmlSettings);
    assertXPath(pXmlSettings, "/w:settings/w:autoHyphenation", "val", u"true");

    // Second paragraph has explicitly enabled hyphenation
    xmlDocUniquePtr pXml = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "/w:document/w:body/w:p[2]/w:pPr/w:suppressAutoHyphens", "val", u"false");

    // Default paragraph style explicitly disables hyphens
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);
    CPPUNIT_ASSERT(pXmlStyles);
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:pPrDefault/w:pPr/w:suppressAutoHyphens",
                "val", u"true");
}

CPPUNIT_TEST_FIXTURE(Test, testStrikeoutGroupShapeText)
{
    loadAndSave("tdf131776_StrikeoutGroupShapeText.docx");
    // tdf#131776: Check if strikeout is used in shape group texts
    xmlDocUniquePtr pXml = parseExport(u"word/document.xml"_ustr);

    // double strike (dstrike)
    //   no "val" attribute
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                      "wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
                      "wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike");
    assertXPathNoAttribute(pXml,
                           "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                           "wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
                           "wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike",
                           "val");
    //   "val" attribute is true, this is used in this test file. However, LO is going to export this element without the "val" attribute
    //   because if the element appears, but without an attribute its "val" true by default.
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                      "wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
                      "wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike");
    assertXPathNoAttribute(pXml,
                           "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                           "wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
                           "wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike",
                           "val");
    //   "val" attribute is false (this was missing, resulting the regression)
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wpg:wgp/"
                "wps:wsp[3]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike",
                "val", u"false");

    // simple strike (strike)
    //   no "val" attribute
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                      "wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
                      "wps:wsp[4]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike");
    assertXPathNoAttribute(pXml,
                           "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                           "wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
                           "wps:wsp[4]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike",
                           "val");
    //   "val" attribute is true, this is used in this test file. However, LO is going to export this element without the "val" attribute
    //   because if the element appears, but without an attribute its "val" true by default.
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                      "wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
                      "wps:wsp[5]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike");
    assertXPathNoAttribute(pXml,
                           "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                           "wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
                           "wps:wsp[5]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike",
                           "val");
    //   "val" attribute is false
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wpg:wgp/"
                "wps:wsp[6]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike",
                "val", u"false");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131539)
{
    loadAndSave("tdf131539.odt");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    //The positions of OLE objects were not exported, check if now it is exported correctly
    xmlDocUniquePtr p_XmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(p_XmlDoc);
    OUString aXmlVal
        = getXPath(p_XmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:object/v:shape", "style");
    // This data was missing
    CPPUNIT_ASSERT(aXmlVal.indexOf("margin-left:139.95") > -1);
}

CPPUNIT_TEST_FIXTURE(Test, testLineWidthRounding)
{
    loadAndSave("tdf126363_LineWidthRounding.docx");
    // tdf#126363: check if line with stays the same after export
    xmlDocUniquePtr pXml = parseExport(u"word/document.xml"_ustr);
    // this was 57240 (it differs from the original 57150, losing the preset line width)
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln",
                "w", u"57150");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108505)
{
    loadAndReload("tdf108505.docx");
    uno::Reference<text::XTextRange> xParagraph = getParagraph(3);
    uno::Reference<text::XTextRange> xText
        = getRun(xParagraph, 1, u"Wrong font when alone on the line"_ustr);

    // Without the fix in place this would have become Times New Roman
    CPPUNIT_ASSERT_EQUAL(u"Trebuchet MS"_ustr, getProperty<OUString>(xText, u"CharFontName"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testRelativeAnchorHeightFromTopMarginHasHeader)
{
    loadAndReload("tdf123324_testRelativeAnchorHeightFromTopMarginHasHeader.docx");
    // tdf#123324 The height was set relative to page print area top,
    // but this was handled relative to page height.
    // Note: page print area top = margin + header height.
    // In this case the header exists.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds", "height", u"2551");
}

CPPUNIT_TEST_FIXTURE(Test, testRelativeAnchorHeightFromTopMarginNoHeader)
{
    loadAndReload("tdf123324_testRelativeAnchorHeightFromTopMarginNoHeader.docx");
    // tdf#123324 The height was set relative from top margin, but this was handled relative from page height.
    // Note: the MSO Word margin = LO margin + LO header height.
    // In this case the header does not exist, so MSO Word margin and LO Writer margin are the same.

    // tdf#123324 The height was set relative to page print area top,
    // but this was handled relative to page height.
    // Note: page print area top = margin + header height.
    // In this case the header does not exist, so OpenDocument and OOXML margins are the same.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds", "height", u"2551");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf64531)
{
    loadAndReload("tdf64531.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    OString sPathToTabs = "/w:document/w:body/w:sdt/w:sdtContent/w:p[2]/w:pPr/w:tabs/"_ostr;
    assertXPath(pXmlDoc, sPathToTabs + "w:tab[1]", "pos", u"720");
    assertXPath(pXmlDoc, sPathToTabs + "w:tab[2]", "pos", u"12950");
}
/* temporarily disabled to get further test results
   The import now uses the dml shape, not the VML fallback.
DECLARE_OOXMLEXPORT_TEST(testVmlShapeTextWordWrap, "tdf97618_testVmlShapeTextWordWrap.docx")
{
    // tdf#97618 The text wrapping of a shape was not handled in a canvas.
    // TODO: fix export too
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    if (!pXmlDoc)
        return;
    // The bound rect of shape will be wider if wrap does not work (the wrong value is 3167).
    assertXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds", "width", "2500");
}
*/

CPPUNIT_TEST_FIXTURE(Test, testVmlLineShapeMirroredX)
{
    // tdf#97517 The "flip:x" was not handled for VML line shapes.
    loadAndSave("tdf97517_testVmlLineShapeMirroredX.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    OUString sStyle = getXPath(
        pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:line",
        "style");
    CPPUNIT_ASSERT(sStyle.indexOf("flip:x") > 0);
}

CPPUNIT_TEST_FIXTURE(Test, testVmlLineShapeMirroredY)
{
    // tdf#137678 The "flip:y" was not handled for VML line shapes.
    loadAndSave("tdf137678_testVmlLineShapeMirroredY.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    OUString sStyle = getXPath(
        pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:line",
        "style");
    CPPUNIT_ASSERT(sStyle.indexOf("flip:y") > 0);
}

CPPUNIT_TEST_FIXTURE(Test, testVmlLineShapeRotated)
{
    loadAndSave("tdf137765_testVmlLineShapeRotated.docx");
    // tdf#137765 The "rotation" (in style attribute) was not handled correctly for VML line shapes.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // it was 1.55pt,279.5pt
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:line",
                "from", u"-9pt,296.75pt");
    // it was 25.5pt,317.8pt
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:line", "to",
                u"36.05pt,300.55pt");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
