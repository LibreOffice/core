/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <xmloff/odffields.hxx>
#include <o3tl/string_view.hxx>

#include <IDocumentMarkAccess.hxx>
#include <IMark.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/ooxmlexport/data/";

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
        return o3tl::ends_with(filename, ".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testFdo47669, "fdo47669.docx")
{
    /*
     * Problem: we created imbalance </w:hyperlink> which shouldn't be there,
     * resulting in loading error: missing last character of hyperlink text
     * and content after it wasn't loaded.
     */
    getParagraph(1, "This is a hyperlink with anchor. Also, this sentence should be seen.");
    getRun(getParagraph(1), 2, "hyperlink with anchor");
    CPPUNIT_ASSERT_EQUAL(OUString("http://www.google.com/#a"), getProperty<OUString>(getRun(getParagraph(1), 2), "HyperLinkURL"));
}

DECLARE_OOXMLEXPORT_TEST(testN789482, "n789482.docx")
{
    // The problem was that w:del was exported before w:hyperlink, resulting in an invalid XML.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    getRun(xParagraph, 1, "Before. ");

    CPPUNIT_ASSERT_EQUAL(OUString("Delete"), getProperty<OUString>(getRun(xParagraph, 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getRun(xParagraph, 2), "IsStart"));

    getRun(xParagraph, 3, "www.test.com");
    CPPUNIT_ASSERT_EQUAL(OUString("http://www.test.com/"), getProperty<OUString>(getRun(xParagraph, 3), "HyperLinkURL"));

    CPPUNIT_ASSERT_EQUAL(OUString("Delete"), getProperty<OUString>(getRun(xParagraph, 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getRun(xParagraph, 4), "IsStart"));

    getRun(xParagraph, 5, " After.");
}

CPPUNIT_TEST_FIXTURE(Test, testBnc834035)
{
    loadAndReload("bnc834035.odt");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(3, getPages());
    // Illustration index had wrong hyperlinks: anchor was using Writer's
    // <seqname>!<index>|sequence syntax, not a bookmark name.
    xmlDocUniquePtr pXmlDoc = parseExport();
    // This was Figure!1|sequence.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:hyperlink", "anchor", "_Toc363553908");
}

DECLARE_OOXMLEXPORT_TEST(testCp1000015, "cp1000015.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Redline and hyperlink end got exported in an incorrect order.
    getParagraph(1, "Hello.");
    getParagraph(2, "http://www.google.com/");
}

CPPUNIT_TEST_FIXTURE(Test, testHyperlineIsEnd)
{
    loadAndReload("hyperlink.docx");
    // Check  that the document.xml contents all the tag properly closed.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // If  document.xml miss any ending tag then parseExport() returns NULL which fail the test case.
    CPPUNIT_ASSERT(pXmlDoc) ;
    // Check hyperlink is properly open.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p/w:hyperlink",1);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo69649)
{
    loadAndReload("fdo69649.docx");
    // The DOCX containing the Table of Contents was not exported with correct page nos
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[21]/w:hyperlink/w:r[5]/w:t", "15");
}

CPPUNIT_TEST_FIXTURE(Test, testFieldFlagO)
{
    loadAndReload("TOC_field_f.docx");
    // This test case is to verify \o flag should come once.
    xmlDocUniquePtr pXmlDoc = parseExport();

    // FIXME "p[2]" will have to be "p[1]", once the TOC import code is fixed
    // not to insert an empty paragraph before TOC.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[1]/w:r[2]/w:instrText", " TOC \\z \\f \\o \"1-3\" \\u \\h");
}

CPPUNIT_TEST_FIXTURE(Test, testTOCFlag_f)
{
    loadAndReload("toc_doc.docx");
    // Export logic for all TOC field flags was enclosed inside
    // if( SwTOXElement::Mark & pTOX->GetCreateType() ) in ww8atr.cxx which gets true for \f,
    // this was the reason if there is \f flag present in original doc then only other flags like
    // \o \h \n used to come after RoundTrip.
    // This test case is to verify even if there is no \f flag in original doc, \h flag is getting
    // preserved after RT.
    xmlDocUniquePtr pXmlDoc = parseExport();

    // FIXME "p[2]" will have to be "p[1]", once the TOC import code is fixed
    // not to insert an empty paragraph before TOC.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[1]/w:r[2]/w:instrText", " TOC \\z \\o \"1-3\" \\u \\h");
}

CPPUNIT_TEST_FIXTURE(Test, testPreserveZfield)
{
    loadAndReload("preserve_Z_field_TOC.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[1]/w:r[2]/w:instrText", " TOC \\z \\f \\o \"1-3\" \\h");
}

CPPUNIT_TEST_FIXTURE(Test, testPreserveWfieldTOC)
{
    loadAndReload("PreserveWfieldTOC.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p/w:r[2]/w:instrText", " TOC \\z \\w \\f \\o \"1-3\" \\h");
}

CPPUNIT_TEST_FIXTURE(Test, testFieldFlagB)
{
    loadAndReload("TOC_field_b.docx");
    // This test case is to verify \b flag.
    xmlDocUniquePtr pXmlDoc = parseExport();

    // FIXME "p[2]" will have to be "p[1]", once the TOC import code is fixed
    // not to insert an empty paragraph before TOC.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[1]/w:r[2]/w:instrText", " TOC \\b \"bookmark111\" \\o \"1-9\" \\h");
}

CPPUNIT_TEST_FIXTURE(Test, testPreserveXfieldTOC)
{
    loadAndReload("PreserveXfieldTOC.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p/w:r[2]/w:instrText", " TOC \\x \\f \\o \"1-3\" \\h");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO77715)
{
    loadAndReload("FDO77715.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:instrText[1]", " TOC \\c ");
}

CPPUNIT_TEST_FIXTURE(Test, testTOCFlag_u)
{
    loadAndReload("testTOCFlag_u.docx");
    // DOCX containing TOC should preserve code field '\u'.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // FIXME "p[2]" will have to be "p[1]", once the TOC import code is fixed
    // not to insert an empty paragraph before TOC.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p/w:r[2]/w:instrText", " TOC \\z \\o \"1-9\" \\u \\h");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo73596_RunInStyle)
{
    loadAndReload("fdo73596_RunInStyle.docx");
    // INDEX should be preserved.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:instrText[1]", " INDEX \\e \"");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo73596_AlphaSeparator)
{
    loadAndReload("fdo73596_AlphaSeparator.docx");
    // INDEX flag \h "A" should be preserved.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:instrText[1]", " INDEX \\h \"A\" \\e \"");
}

CPPUNIT_TEST_FIXTURE(Test, testCaption1)
{
    loadAndReload("EquationAsScientificNumbering.docx");
    // fdo#74431 : This test case is to verify the Captions are coming properly
    // earlier it was coming as "SEQ "scientific"\*ROMAN now it is SEQ scientific\* ROMAN"

    xmlDocUniquePtr pXmlDoc = parseExport();

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[3]/w:instrText", " SEQ scientific \\* ROMAN ");
}

CPPUNIT_TEST_FIXTURE(Test, testCaption2)
{
    loadAndReload("EquationWithAboveAndBelowCaption.docx");
    // fdo#72563 : There was a problem that in case of TOC,PAGEREF field tag was not preserved during Roundtrip
    // This test case is to verify that PAGEREF tag is coming with proper values inside <hyperlink> tag.
    xmlDocUniquePtr pXmlDoc = parseExport();

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[5]/w:r[3]/w:instrText", " SEQ Equation \\* ARABIC ");
}

CPPUNIT_TEST_FIXTURE(Test, testCaption3)
{
    loadAndReload("FigureAsLabelPicture.docx");
    // fdo#72563 : There was a problem that in case of TOC,PAGEREF field tag was not preserved during Roundtrip
    // This test case is to verify that PAGEREF tag is coming with proper values inside <hyperlink> tag.
    xmlDocUniquePtr pXmlDoc = parseExport();

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:instrText", " SEQ picture \\* ARABIC ");
}

CPPUNIT_TEST_FIXTURE(Test, testCaption4)
{
    loadAndReload("TableWithAboveCaptions.docx");
    // fdo#72563 : There was a problem that in case of TOC,PAGEREF field tag was not preserved during Roundtrip
    // This test case is to verify that PAGEREF tag is coming with proper values inside <hyperlink> tag.
    xmlDocUniquePtr pXmlDoc = parseExport();

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:instrText", " SEQ Table \\* ARABIC ");
}

CPPUNIT_TEST_FIXTURE(Test, testFooterContainHyperlink)
{
    loadAndReload("footer-contain-hyperlink.docx");
    // Problem is that footer1.xml.rels contains the empty
    // Target due to which the file get corrupted
    // in MS Office 2007.
    // Check for footer1.xml.rels file.
    xmlDocUniquePtr pXmlRels = parseExport("word/_rels/footer1.xml.rels");
    // Check the value of Target which is http://www.google.com/.
    assertXPath(pXmlRels,"/rels:Relationships/rels:Relationship","Target","http://www.google.com/");
}

CPPUNIT_TEST_FIXTURE(Test, testAlphabeticalIndex_MultipleColumns)
{
    loadAndReload("alphabeticalIndex_MultipleColumns.docx");
    // Bug :: fdo#73596
    /*
     * Index with multiple columns was not imported correctly and
     * hence not exported correctly...
     * The column count is given by the \c switch.
     * If the column count is explicitly specified,
     * MS Office adds section breaks before and after the Index.
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:instrText", " INDEX \\c \"4\"\\e \"");

    // check for section breaks after and before the Index Section
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:sectPr/w:type","val","continuous");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:pPr/w:sectPr/w:type","val","continuous");
    // check for "w:space" attribute for the columns in Section Properties
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:pPr/w:sectPr/w:cols","space","720");
}

CPPUNIT_TEST_FIXTURE(Test, testPageref)
{
    loadAndReload("testPageref.docx");
    // fdo#72563 : There was a problem that in case of TOC,PAGEREF field tag was not preserved during Roundtrip
    // This test case is to verify that PAGEREF tag is coming with proper values inside <hyperlink> tag.
    xmlDocUniquePtr pXmlDoc = parseExport();

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[2]/w:hyperlink/w:r[3]/w:instrText", "PAGEREF _Toc355095261 \\h");
}

CPPUNIT_TEST_FIXTURE(Test, testAlphabeticalIndex_AutoColumn)
{
    loadAndReload("alphabeticalIndex_AutoColumn.docx");
    // Bug :: fdo#73596
    /*
     * When the columns in Index are 0; i.e not specified by the
     * "\c" switch, don't write back '\c "0"' or the section breaks
     * before and after the Index Context
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:instrText", " INDEX \\e \"");

    // check for section break doesn't appear for any paragraph
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:sectPr", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testIndexFieldFlagF)
{
    loadAndReload("IndexFieldFlagF.docx");
    // This test case is to verify the Index field flag '\f' with some
    // Specific Entry Type (ex. "Syn" in our case).
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // We check the Index field flag '\f'.
    // Note: no syntax error any more (extra quotation mark between "Syn" and \e)
    assertXPathContent(pXmlDoc, "/w:document[1]/w:body[1]/w:p[4]/w:r[2]/w:instrText[1]", " INDEX \\c \"2\"\\f \"Syn\"\\e \"");
    // XE entries lost their \f "Syn" before
    assertXPathContent(pXmlDoc, "/w:document[1]/w:body[1]/w:p[17]/w:r[21]/w:instrText[1]", " XE \"formatting\" \\f \"Syn\" ");
    assertXPathContent(pXmlDoc, "/w:document[1]/w:body[1]/w:p[17]/w:r[29]/w:instrText[1]", " XE \"choosing:aaaa\" \\f \"Syn\" ");
}

CPPUNIT_TEST_FIXTURE(Test, testBibliography)
{
    loadAndReload("FDO75133.docx");
    xmlDocUniquePtr pXmlDoc = parseExport();

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p/w:r[2]/w:instrText", " BIBLIOGRAPHY ");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:docPartObj/w:docPartGallery", "val", "Bibliographies");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:docPartObj/w:docPartUnique", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testGenericTextField)
{
    loadAndReload("Unsupportedtextfields.docx");
    // fdo#75158 : This test case is to verify the unsupported textfields are exported properly.

    xmlDocUniquePtr pXmlDoc = parseExport();
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc,"/w:document/w:body/w:p[2]/w:r[2]/w:instrText");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii(reinterpret_cast<char*>((pXmlNode->children[0]).content));
    CPPUNIT_ASSERT(contents.match(" PRINTDATE "));
    xmlXPathFreeObject(pXmlObj);
}

CPPUNIT_TEST_FIXTURE(Test, test_FieldType)
{
    loadAndReload("99_Fields.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Checking for three field types (BIBLIOGRAPHY, BIDIOUTLINE, CITATION) in sequence
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:instrText");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:instrText");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:sdt/w:sdtContent/w:r[2]/w:instrText");
}

CPPUNIT_TEST_FIXTURE(Test, testCitation)
{
    loadAndReload("FDO74775.docx");
    xmlDocUniquePtr pXmlDoc = parseExport();
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:sdt/w:sdtContent/w:r[2]/w:instrText", " CITATION Kra06 \\l 1033 ");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:sdt/w:sdtContent/w:r[4]/w:t", "(Kramer & Chen, 2006)");
}

CPPUNIT_TEST_FIXTURE(Test, testHyperLinkTagEnded)
{
    loadAndReload("fdo76316.docx");
    /* XML tag <w:hyperlink> was not getting closed when its inside another
     * <w:hyperlink> tag.
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[2]/w:tc[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl[1]/w:tr[7]/w:tc[1]/w:tbl[1]/w:tr[2]/w:tc[6]/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[1]/w:hyperlink[1]/w:hyperlink[1]",1);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO76163 )
{
    loadAndReload("fdo76163.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    //docx file after RT is getting corrupted.
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[2]/w:hyperlink/w:r[10]/w:fldChar", "fldCharType", "end" );
}

CPPUNIT_TEST_FIXTURE(Test, testFDO78659)
{
    loadAndReload("fdo78659.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[1]/w:hyperlink[1]/w:r[3]/w:fldChar[1]", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO78654 )
{
    loadAndReload("fdo78654.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // In case of two "Hyperlink" tags in one paragraph and one of them
    // contains "PAGEREF" field then field end tag was missing from hyperlink.
    assertXPath ( pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[2]/w:hyperlink[3]/w:r[5]/w:fldChar", "fldCharType", "end" );
}


CPPUNIT_TEST_FIXTURE(Test, testfdo78599)
{
    loadAndReload("fdo78599.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    //docx file after RT is getting corrupted.
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink/w:r[6]/w:fldChar", "fldCharType", "end" );

    // Check for automatic hyphenation
    xmlDocUniquePtr pSettingsXml = parseExport("word/settings.xml");
    // This failed as w:settings had no w:autoHyphenation child.
    assertXPath(pSettingsXml, "/w:settings/w:autoHyphenation");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo78886)
{
    loadAndReload("fdo78886.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[2]/w:tr[1]/w:tc[1]/w:p[1]/w:hyperlink[1]/w:r[2]/w:fldChar[1]", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo78910)
{
    loadAndReload("fdo78910.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // This is to ensure that the fld starts and ends inside a hyperlink...
    assertXPath ( pXmlDoc, "//w:hyperlink[2]/w:r[1]/w:fldChar", "fldCharType", "begin" );
    assertXPath ( pXmlDoc, "//w:hyperlink[2]/w:r[5]/w:fldChar", "fldCharType", "end" );
}

// FIXME: During this test a pure VML shape get converted to DML and crash at verifying.
// CPPUNIT_TEST_FIXTURE(Test, testFDO78590)
// {
//     loadAndReload("FDO78590.docx");
//     xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
//
//     // This is to ensure that the fld starts and ends inside a hyperlink...
//     assertXPath ( pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "w", "9851" );
//     assertXPath ( pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "h", "1669" );
// }

CPPUNIT_TEST_FIXTURE(Test, testSdtCitationRun)
{
    loadAndReload("sdt-citation-run.docx");
    // The problem was that the SDT was around the whole paragraph, not only around the citation field.
    xmlDocUniquePtr pXmlDoc = parseExport();
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:t", "Before sdt.");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r/w:instrText", " CITATION BBC11 \\l 1033 ");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:t", "After sdt.");
}

CPPUNIT_TEST_FIXTURE(Test, testParagraphSdt)
{
    loadAndReload("paragraph-sdt.docx");
    // The problem was that the SDT was around the run only, not the whole paragraph.
    xmlDocUniquePtr pXmlDoc = parseExport();
    // The path to w:sdt contained a w:p.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:sdt");
}

CPPUNIT_TEST_FIXTURE(Test, testSdt2Run)
{
    loadAndReload("sdt-2-para.docx");
    xmlDocUniquePtr pXmlDoc = parseExport();
    // The problem was that <w:sdt> was closed after "first", not after "second", so the second assert failed.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[1]/w:r/w:t", "first");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[2]/w:r/w:t", "second");
    // Make sure the third paragraph is still outside <w:sdt>.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r/w:t", "third");
}

CPPUNIT_TEST_FIXTURE(Test, test2Id)
{
    loadAndReload("2-id.docx");
    xmlDocUniquePtr pXmlDoc = parseExport();
    // This was 2, but only one w:id is allowed.
    assertXPath(pXmlDoc, "//w:sdtPr/w:id", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTableStart2Sdt)
{
    loadAndReload("table-start-2-sdt.docx");
    xmlDocUniquePtr pXmlDoc = parseExport();
    // w:docPartGallery should be a child of <w:docPartObj>, make sure it's not a child of w:text.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:text/w:docPartGallery", 0);
}

DECLARE_OOXMLEXPORT_TEST(testSdtDateDuplicate, "sdt-date-duplicate.docx")
{
    if (xmlDocUniquePtr pXmlDoc = parseExport())
    {
        // Single <w:sdt> was exported as 2 <w:sdt> elements.
        assertXPath(pXmlDoc, "//w:sdt", 1);
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
        uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xCell, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParagraphs = xParagraphsAccess->createEnumeration();
        uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
        uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);
        OUString aPortionType;
        xTextPortion->getPropertyValue("TextPortionType") >>= aPortionType;
        CPPUNIT_ASSERT_EQUAL(OUString("ContentControl"), aPortionType);
        uno::Reference<text::XTextContent> xContentControl;
        xTextPortion->getPropertyValue("ContentControl") >>= xContentControl;
        uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
        bool bDate{};
        xContentControlProps->getPropertyValue("Date") >>= bDate;
        CPPUNIT_ASSERT(bDate);
        uno::Reference<container::XEnumerationAccess> xContentControlEnumAccess(xContentControl, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xContentControlEnum = xContentControlEnumAccess->createEnumeration();
        uno::Reference<text::XTextRange> xTextPortionRange(xContentControlEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("4/26/2012"), xTextPortionRange->getString());
    }
    else
    {
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);
        SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
        IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pMarkAccess->getAllMarksCount());

        ::sw::mark::IDateFieldmark* pFieldmark
            = dynamic_cast<::sw::mark::IDateFieldmark*>(*pMarkAccess->getAllMarksBegin());
        CPPUNIT_ASSERT(pFieldmark);
        CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());
        CPPUNIT_ASSERT_EQUAL(OUString("4/26/2012"), pFieldmark->GetContent());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testFdo81492)
{
    loadAndReload("fdo81492.docx");
    xmlDocUniquePtr pXmlDoc = parseExport();
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[6]/w:instrText", "ADDIN EN.CITE.DATA");
}

CPPUNIT_TEST_FIXTURE(Test, testEditTime)
{
    loadAndReload("fdo81341.docx");
    /* Issue was LO was not able to Import and Export EditTime in seconds format.
     * It was supporting Time in "HH:MM" format. But if DOCX contains Time in seconds,
     * then LO was not able to display time in "HH:MM:SS" format.
     * While exporting LO was writing plain text instead of field entry.
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    //Ensure that EditTime is written inside w:fldChar in "HH:MM:SS" format.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:fldChar", "fldCharType", "begin");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[3]/w:fldChar", "fldCharType", "separate");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:t", "00:05");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:fldChar", "fldCharType", "end");
}

CPPUNIT_TEST_FIXTURE(Test, testFlyFieldmark)
{
    loadAndReload("fly_fieldmark.fodt");
    // the problem was that the flys were written after the field start
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // run 1 contains 2 shapes, one was at-page, one was at-char
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent", 2);
    // run 2 contains the field start
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:fldChar", "fldCharType", "begin");
    // run 3 contains the field instruction text
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:instrText", " FORMTEXT ");
}

CPPUNIT_TEST_FIXTURE(Test, testFdo81945)
{
    loadAndReload("fdo81945.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "//w:sdt//w:sdt", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testfdo82123)
{
    loadAndReload("fdo82123.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // make sure there is only one run inside first SDT after RT as in the Original file.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[2]/w:p/w:sdt[1]/w:sdtContent/w:r",1);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtBeforeField)
{
    loadAndReload("sdt-before-field.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Make sure the field doesn't sneak inside the SDT: the SDT should contain only a single run (there were 6 ones).
    assertXPath(pXmlDoc, "//w:sdt/w:sdtContent/w:r", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testfdo81946)
{
    loadAndReload("fdo81946.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/header1.xml");
    // make sure AlternateContent should not present in sdt
    assertXPath(pXmlDoc, "/w:hdr[1]/w:p[1]/w:sdt[1]/w:sdtContent[1]/w:r[2]/mc:AlternateContent[1]",0);
}

CPPUNIT_TEST_FIXTURE(Test, testfdo82492)
{
    loadAndReload("fdo82492.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // make sure there is only one run inside first SDT after RT as in the Original file.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt[1]/w:sdtContent/w:r",1);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtHeader)
{
    loadAndReload("sdt-header.docx");
    // Problem was that w:sdt elements in headers were lost on import.
    xmlDocUniquePtr pXmlDoc = parseExport("word/header2.xml");
    // This was 0, w:sdt (and then w:date) was missing.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:date", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtCompanyMultipara)
{
    loadAndReload("sdt-company-multipara.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Here is just a simple text node, so there should be either one or zero paragraph
    // (in this case sdt element is inside paragraph)
    assertXPath(pXmlDoc, "//w:sdtContent/w:p", 0);
    assertXPath(pXmlDoc, "//w:sdtContent/w:r", 1);
}

DECLARE_OOXMLEXPORT_TEST(testFixedDateFields, "fixed-date-field.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xField(xFields->nextElement(), uno::UNO_QUERY);

    // Check fixed property was imported and date value was parsed correctly
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xField, "IsFixed"));
    css::util::DateTime date = getProperty<css::util::DateTime>(xField, "DateTimeValue");
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(24), date.Day);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(7), date.Month);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2014), date.Year);

    if (xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml"))
    {
        // Previously, fixed fields were exported as static text ("Date (fixed)")
        // Check they are now exported correctly as fldChar with fldLock attribute
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:fldChar", "fldLock", "true");
    }
}

DECLARE_OOXMLEXPORT_TEST(testToxmarkHyperlink, "toxmarkhyperlink.fodt")
{
    // test that export doesn't assert with overlapping fields / hyperlink attr
}

DECLARE_OOXMLEXPORT_TEST(testOO34469, "ooo34469-1.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (xmlDocUniquePtr pXmlDoc = parseExport())
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink[1]", "anchor", "2.9.2.Creating_New_files|outline");
}

DECLARE_OOXMLEXPORT_TEST(testOO39845, "ooo39845-7.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (xmlDocUniquePtr pXmlDoc = parseExport())
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink[1]", "anchor", "Figure4|graphic");
}

DECLARE_OOXMLEXPORT_TEST( testTdf85161, "tdf85161.docx" )
{
    CPPUNIT_ASSERT_EQUAL(OUString("Symbol"), getProperty<OUString>(getRun(getParagraph(1), 1), "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString(u'\x5e'),getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf66401)
{
    loadAndReload("tdf66401.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w:rFonts", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w:rFonts", "ascii", "Arial Black");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w:sz", "val", "24");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[9]/w:rPr/w:rFonts", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[9]/w:rPr/w:rFonts", "ascii", "Arial Black");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[9]/w:rPr/w:sz", "val", "24");
}

DECLARE_OOXMLEXPORT_TEST( testDateFieldInShape, "date_field_in_shape.docx" )
{
    // This was crashed on export.
    if (mbExported)
    {
        uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
        uno::Reference<text::XText> xShapeText = xShape->getText();
        uno::Reference<beans::XPropertySet> xTextPortion(getRun(getParagraphOfText(1, xShapeText), 1), uno::UNO_QUERY);
        OUString aPortionType;
        xTextPortion->getPropertyValue("TextPortionType") >>= aPortionType;
        CPPUNIT_ASSERT_EQUAL(OUString("ContentControl"), aPortionType);
        uno::Reference<text::XTextContent> xContentControl;
        xTextPortion->getPropertyValue("ContentControl") >>= xContentControl;
        uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
        bool bDate{};
        xContentControlProps->getPropertyValue("Date") >>= bDate;
        CPPUNIT_ASSERT(bDate);
        uno::Reference<container::XEnumerationAccess> xContentControlEnumAccess(xContentControl, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xContentControlEnum = xContentControlEnumAccess->createEnumeration();
        uno::Reference<text::XTextRange> xTextPortionRange(xContentControlEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Click here to enter a date."), xTextPortionRange->getString());
    }
    else
    {
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);
        SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
        IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pMarkAccess->getAllMarksCount());

        ::sw::mark::IDateFieldmark* pFieldmark
            = dynamic_cast<::sw::mark::IDateFieldmark*>(*pMarkAccess->getAllMarksBegin());
        CPPUNIT_ASSERT(pFieldmark);
        CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());
        CPPUNIT_ASSERT_EQUAL(OUString("Click here to enter a date."), pFieldmark->GetContent());
    }
}

DECLARE_OOXMLEXPORT_TEST( testDateFieldAtEndOfParagraph, "date_field_at_end_of_paragraph.docx" )
{
    // Additional line end was added by import and it was crashed on export
    if (mbExported)
    {
        uno::Reference<beans::XPropertySet> xTextPortion(getRun(getParagraph(2), 1), uno::UNO_QUERY);
        OUString aPortionType;
        xTextPortion->getPropertyValue("TextPortionType") >>= aPortionType;
        CPPUNIT_ASSERT_EQUAL(OUString("ContentControl"), aPortionType);
        uno::Reference<text::XTextContent> xContentControl;
        xTextPortion->getPropertyValue("ContentControl") >>= xContentControl;
        uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
        bool bDate{};
        xContentControlProps->getPropertyValue("Date") >>= bDate;
        CPPUNIT_ASSERT(bDate);
        uno::Reference<container::XEnumerationAccess> xContentControlEnumAccess(xContentControl, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xContentControlEnum = xContentControlEnumAccess->createEnumeration();
        uno::Reference<text::XTextRange> xTextPortionRange(xContentControlEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Click here to enter a date."), xTextPortionRange->getString());
    }
    else
    {
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);
        SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
        IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pMarkAccess->getAllMarksCount());

        ::sw::mark::IDateFieldmark* pFieldmark
            = dynamic_cast<::sw::mark::IDateFieldmark*>(*pMarkAccess->getAllMarksBegin());
        CPPUNIT_ASSERT(pFieldmark);
        CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());
        CPPUNIT_ASSERT_EQUAL(OUString("Click here to enter a date."), pFieldmark->GetContent());
    }
}

DECLARE_OOXMLEXPORT_TEST(testDropDownFieldEntryLimit, "tdf126792.odt" )
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // In MSO, there is a limit of 25 for the items in a drop-down form field.
    // So we truncate the list of items to not exceed this limit.

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    ::sw::mark::IFieldmark* pFieldmark
          = dynamic_cast<::sw::mark::IFieldmark*>(*pMarkAccess->getAllMarksBegin());
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDROPDOWN), pFieldmark->GetFieldname());

    const sw::mark::IFieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
    auto pListEntries = pParameters->find(ODF_FORMDROPDOWN_LISTENTRY);
    CPPUNIT_ASSERT(bool(pListEntries != pParameters->end()));
    css::uno::Sequence<OUString> vListEntries;
    pListEntries->second >>= vListEntries;
    if (!mbExported)
        CPPUNIT_ASSERT_EQUAL(sal_Int32(26), vListEntries.getLength());
    else
        CPPUNIT_ASSERT_EQUAL(sal_Int32(25), vListEntries.getLength());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132185)
{
    loadAndReload("tdf132185.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/footer1.xml");
    // Since the default (without xml:space attribute) is to ignore leading and trailing spaces,
    // " PAGE \\* roman " will get imported as "PAGE \\* roman". This is also valid, and must be
    // treated accordingly. "roman" was ignored before the fix, exporting only " PAGE ".
    assertXPathContent(pXmlDoc, "/w:ftr/w:p/w:r[2]/w:instrText", " PAGE \\* roman ");
}

CPPUNIT_TEST_FIXTURE(Test, testConditionalText)
{
    loadAndReload("conditional-text.fodt");
    // Load a document which has a conditional text field in it.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    std::u16string_view aExpected(u" IF 1 < 2 \"True\" \"False\"");

    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: xmlXPathNodeSetGetLength(pXmlNodes) > 0
    // - In <...>, XPath '/w:document/w:body/w:p/w:r[2]/w:instrText' not found
    // i.e. the field was lost on export.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:instrText", OUString(aExpected));
}

DECLARE_OOXMLEXPORT_TEST(testTdf142464_ampm, "tdf142464_ampm.docx")
{
    css::uno::Reference<css::text::XTextFieldsSupplier> xTextFieldsSupplier(
        mxComponent, css::uno::UNO_QUERY_THROW);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    auto xFields(xFieldsAccess->createEnumeration());
    css::uno::Reference<css::text::XTextField> xField(xFields->nextElement(),
                                                      css::uno::UNO_QUERY_THROW);

    // Without the fix in place, this would have failed with:
    //   - Expected: 12:32 PM
    //   - Actual  : 12:32 a12/p12
    CPPUNIT_ASSERT_EQUAL(OUString("12:32 PM"), xField->getPresentation(false));

    if (xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml"))
    {
        // Without the fix in place, this would have failed with:
        //   - Expected:  DATE \@"H:mm\ AM/PM"
        //   - Actual  :  DATE \@"H:mm' a'M'/p'M"
        // i.e., the AM/PM would be treated as literal 'a' and 'p' followed by a month code
        assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:instrText",
                           " DATE \\@\"H:mm\\ AM/PM\" ");
    }
}

DECLARE_OOXMLEXPORT_TEST( testSdtDatePicker, "test_sdt_datepicker.docx" )
{
    // Check that roundtrip for date picker field does not lose essential data
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return; // initial import, no further checks

    // Placeholder is here
    OUString sDocPart = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:placeholder/w:docPart", "val");
    CPPUNIT_ASSERT_EQUAL(OUString("DefaultPlaceholder_-1854013437"), sDocPart);

    // Ensure that we have data binding stuff
    OUString sDBprefix = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dataBinding", "prefixMappings");
    CPPUNIT_ASSERT_EQUAL(OUString("xmlns:ns0='http://schemas.microsoft.com/vsto/samples' "), sDBprefix);

    OUString sDBxpath = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dataBinding", "xpath");
    CPPUNIT_ASSERT_EQUAL(OUString("/ns0:employees[1]/ns0:employee[1]/ns0:hireDate[1]"), sDBxpath);

    OUString sDBstoreid = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dataBinding", "storeItemID");
    CPPUNIT_ASSERT_EQUAL(OUString("{241A8A02-7FFD-488D-8827-63FBE74E8BC9}"), sDBstoreid);

    OUString sColor = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w15:color", "val");
    CPPUNIT_ASSERT_EQUAL(OUString("008000"), sColor);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104823)
{
    // Test how we can roundtrip sdt plain text with databindings support
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf104823.docx";
    loadURL(aURL, nullptr);

    css::uno::Reference<css::text::XTextFieldsSupplier> xTextFieldsSupplier(
        mxComponent, css::uno::UNO_QUERY_THROW);
    auto xFields(xTextFieldsSupplier->getTextFields()->createEnumeration());

    // FIXME: seems order of fields is different than in source document
    // so feel free to modify testcase if order is changed

    // First field: content from core properties
    uno::Reference<text::XTextField> xField1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    // Check field value (it should be value from data source) and set new
    CPPUNIT_ASSERT_EQUAL(OUString("True Core Property Value"), xField1->getPresentation(false));
    uno::Reference<beans::XPropertySet> xField1Props(xField1, uno::UNO_QUERY);
    xField1Props->setPropertyValue("Content", uno::Any(OUString("New Core Property Value")));

    // Third field: content from custom properties
    uno::Reference<text::XTextField> xField2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    // Check field value (it should be value from data source) and set new
    CPPUNIT_ASSERT_EQUAL(OUString("True Custom XML Value"), xField2->getPresentation(false));
    uno::Reference<beans::XPropertySet> xField2Props(xField2, uno::UNO_QUERY);
    xField2Props->setPropertyValue("Content", uno::Any(OUString("New Custom XML Value")));

    // Second field: content from extended properties
    uno::Reference<text::XTextField> xField3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(!xFields->hasMoreElements());
    // Check field value (it should be value from data source) and set new
    CPPUNIT_ASSERT_EQUAL(OUString("True Extended Property Value"), xField3->getPresentation(false));
    uno::Reference<beans::XPropertySet> xField3Props(xField3, uno::UNO_QUERY);
    xField3Props->setPropertyValue("Content", uno::Any(OUString("New Extended Property Value")));

    // Save and check saved data
    save("Office Open XML Text", maTempFile);
    mbExported = true;
    xmlDocUniquePtr pXmlCustomPropsDoc = parseExport("customXml/item1.xml");
    CPPUNIT_ASSERT(pXmlCustomPropsDoc);
    // FIXME: strange it won't run simple /employees/employee/name xpath query. Does not like namespaces?
    assertXPathContent(pXmlCustomPropsDoc, "//*/*[local-name()='name']", "New Custom XML Value");

    xmlDocUniquePtr pXmlAppPropsDoc = parseExport("docProps/app.xml");
    CPPUNIT_ASSERT(pXmlAppPropsDoc);
    // TODO: extended properties are not written yet
    assertXPathContent(pXmlAppPropsDoc, "//*/*[local-name()='Company']", "True Extended Property Value");

    xmlDocUniquePtr pXmlCorePropsDoc = parseExport("docProps/core.xml");
    CPPUNIT_ASSERT(pXmlCorePropsDoc);
    // TODO: core properties are not written yet
    assertXPathContent(pXmlCorePropsDoc, "/cp:coreProperties/dc:creator", "True Core Property Value");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
