/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <unotools/tempfile.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/Hatch.hpp>

#include <string>

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
        return (OString(filename).endsWith(".docx"));
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

DECLARE_OOXMLEXPORT_TEST(testBnc834035, "bnc834035.odt")
{
    // Illustration index had wrong hyperlinks: anchor was using Writer's
    // <seqname>!<index>|sequence syntax, not a bookmark name.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    // This was Figure!1|sequence.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:hyperlink", "anchor", "_Toc363553908");
}

DECLARE_OOXMLEXPORT_TEST(testCp1000015, "cp1000015.odt")
{
    // Redline and hyperlink end got exported in an incorrect order.
    getParagraph(1, "Hello.");
    getParagraph(2, "http://www.google.com/");
}

DECLARE_OOXMLEXPORT_TEST(testHyperlineIsEnd, "hyperlink.docx")
{
    // Check  that the document.xml contents all the tag properly closed.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // If  document.xml miss any ending tag then parseExport() returns NULL which fail the test case.
    CPPUNIT_ASSERT(pXmlDoc) ;
    // Check hyperlink is properly open.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p/w:hyperlink",1);
}

DECLARE_OOXMLEXPORT_TEST(testFdo69649, "fdo69649.docx")
{
    // The DOCX containing the Table of Contents was not exported with correct page nos
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[21]/w:hyperlink/w:r[5]/w:t", "15");
}

DECLARE_OOXMLEXPORT_TEST(testFieldFlagO,"TOC_field_f.docx")
{
    // This test case is to verify \o flag should come once.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    // FIXME "p[2]" will have to be "p[1]", once the TOC import code is fixed
    // not to insert an empty paragraph before TOC.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:instrText", " TOC \\z \\f \\o \"1-3\" \\u \\h");
}

DECLARE_OOXMLEXPORT_TEST(testTOCFlag_f, "toc_doc.docx")
{
    // Export logic for all TOC field flags was enclosed inside
    // if( nsSwTOXElement::TOX_MARK & pTOX->GetCreateType() ) in ww8atr.cxx which gets true for \f,
    // this was the reason if there is \f flag present in original doc then only other flags like
    // \o \h \n used to come after RoundTrip.
    // This test case is to verify even if there is no \f flag in original doc, \h flag is getting
    // preserved after RT.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    // FIXME "p[2]" will have to be "p[1]", once the TOC import code is fixed
    // not to insert an empty paragraph before TOC.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:instrText", " TOC \\z \\o \"1-3\" \\u \\h");
}

DECLARE_OOXMLEXPORT_TEST(testPreserveZfield,"preserve_Z_field_TOC.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[1]/w:r[2]/w:instrText", " TOC \\z \\f \\o \"1-3\" \\h");
}

DECLARE_OOXMLEXPORT_TEST(testPreserveWfieldTOC, "PreserveWfieldTOC.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p/w:r[2]/w:instrText", " TOC \\z \\w \\f \\o \"1-3\" \\h");
}

DECLARE_OOXMLEXPORT_TEST(testFieldFlagB,"TOC_field_b.docx")
{
    // This test case is to verify \b flag.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    // FIXME "p[2]" will have to be "p[1]", once the TOC import code is fixed
    // not to insert an empty paragraph before TOC.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:instrText", " TOC \\b \"bookmark111\" \\o \"1-9\" \\h");
}

DECLARE_OOXMLEXPORT_TEST(testPreserveXfieldTOC, "PreserveXfieldTOC.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p/w:r[2]/w:instrText", " TOC \\x \\f \\o \"1-3\" \\h");
}

DECLARE_OOXMLEXPORT_TEST(testFDO77715,"FDO77715.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:instrText[1]", " TOC \\c ");
}

DECLARE_OOXMLEXPORT_TEST(testTOCFlag_u,"testTOCFlag_u.docx")
{
    // DOCX containing TOC should preserve code field '\u'.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // FIXME "p[2]" will have to be "p[1]", once the TOC import code is fixed
    // not to insert an empty paragraph before TOC.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p/w:r[2]/w:instrText", " TOC \\z \\o \"1-9\" \\u \\h");
}

DECLARE_OOXMLEXPORT_TEST(testfdo73596_RunInStyle,"fdo73596_RunInStyle.docx")
{
    // INDEX should be preserved.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:instrText[1]", " INDEX \\e \"");
}

DECLARE_OOXMLEXPORT_TEST(testfdo73596_AlphaSeparator,"fdo73596_AlphaSeparator.docx")
{
    // INDEX flag \h "A" should be preserved.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:instrText[1]", " INDEX \\h \"A\" \\e \"");
}

DECLARE_OOXMLEXPORT_TEST(testCaption1, "EquationAsScientificNumbering.docx")
{
    // fdo#74431 : This test case is to verify the Captions are coming properly
    // earlier it was coming as "SEQ "scientific"\*ROMAN now it is SEQ scientific\* ROMAN"

    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[3]/w:instrText", " SEQ scientific \\* ROMAN ");
}

DECLARE_OOXMLEXPORT_TEST(testCaption2, "EquationWithAboveAndBelowCaption.docx")
{
    // fdo#72563 : There was a problem that in case of TOC,PAGEREF field tag was not preserved during Roundtrip
    // This test case is to verify that PAGEREF tag is coming with proper values inside <hyperlink> tag.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[5]/w:r[3]/w:instrText", " SEQ Equation \\* ARABIC ");
}

DECLARE_OOXMLEXPORT_TEST(testCaption3, "FigureAsLabelPicture.docx")
{
    // fdo#72563 : There was a problem that in case of TOC,PAGEREF field tag was not preserved during Roundtrip
    // This test case is to verify that PAGEREF tag is coming with proper values inside <hyperlink> tag.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:instrText", " SEQ picture \\* ARABIC ");
}

DECLARE_OOXMLEXPORT_TEST(testCaption4, "TableWithAboveCaptions.docx")
{
    // fdo#72563 : There was a problem that in case of TOC,PAGEREF field tag was not preserved during Roundtrip
    // This test case is to verify that PAGEREF tag is coming with proper values inside <hyperlink> tag.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:instrText", " SEQ Table \\* ARABIC ");
}

DECLARE_OOXMLEXPORT_TEST(testFooterContainHyperlink,"footer-contain-hyperlink.docx")
{
    // Problem is that footer1.xml.rels contains the empty
    // Target due to which the file get corrupted
    // in MS Office 2007.
    // Check for footer1.xml.rels file.
    xmlDocPtr pXmlRels = parseExport("word/_rels/footer1.xml.rels");
    if (!pXmlRels)
        return;
    // Check the value of Target which is http://www.google.com/.
    assertXPath(pXmlRels,"/rels:Relationships/rels:Relationship","Target","http://www.google.com/");
}

DECLARE_OOXMLEXPORT_TEST(testAlphabeticalIndex_MultipleColumns,"alphabeticalIndex_MultipleColumns.docx")
{
    // Bug :: fdo#73596
    /*
     * Index with multiple columns was not imported correctly and
     * hence not exported correctly...
     * The column count is given by the \c switch.
     * If the column count is explicitly specified,
     * MS Office adds section breaks before and after the Index.
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:instrText", " INDEX \\c \"4\"\\e \"");

    // check for section breaks after and before the Index Section
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:sectPr/w:type","val","continuous");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:sectPr/w:type","val","continuous");
    // check for "w:space" attribute for the columns in Section Properties
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:sectPr/w:cols/w:col[1]","space","720");
}

DECLARE_OOXMLEXPORT_TEST(testPageref, "testPageref.docx")
{
    // fdo#72563 : There was a problem that in case of TOC,PAGEREF field tag was not preserved during Roundtrip
    // This test case is to verify that PAGEREF tag is coming with proper values inside <hyperlink> tag.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[2]/w:hyperlink/w:r[3]/w:instrText", "PAGEREF _Toc355095261 \\h");
}

DECLARE_OOXMLEXPORT_TEST(testAlphabeticalIndex_AutoColumn,"alphabeticalIndex_AutoColumn.docx")
{
    // Bug :: fdo#73596
    /*
     * When the columns in Index are 0; i.e not specified by the
     * "\c" switch, don't write back '\c "0"' or the section breaks
     * before and after the Index Context
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:instrText", " INDEX \\e \"");

    // check for section break doesn't appear for any paragraph
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:sectPr", 0);
}

DECLARE_OOXMLEXPORT_TEST(testIndexFieldFlagF,"IndexFieldFlagF.docx")
{
    // This test case is to verify the Index field flag '\f' with some
    // Specific Entry Type (ex. "Syn" in our case).
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // We check the Index field flag '\f'.
    assertXPathContent(pXmlDoc, "/w:document[1]/w:body[1]/w:p[4]/w:r[2]/w:instrText[1]", " INDEX \\c \"2\"\\f \"Syn\" \" \\e \"");
}

DECLARE_OOXMLEXPORT_TEST(testBibliography,"FDO75133.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p/w:r[2]/w:instrText", " BIBLIOGRAPHY ");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:docPartObj/w:docPartGallery", "val", "Bibliographies");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:docPartObj/w:docPartUnique", 1);
}

DECLARE_OOXMLEXPORT_TEST(testGenericTextField, "Unsupportedtextfields.docx")
{
    // fdo#75158 : This test case is to verify the unsupported textfields are exported properly.

    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc,"/w:document/w:body/w:p[2]/w:r[2]/w:instrText");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii(reinterpret_cast<char*>((pXmlNode->children[0]).content));
    CPPUNIT_ASSERT(contents.match("PRINTDATE   \\* MERGEFORMAT"));
    xmlXPathFreeObject(pXmlObj);
}

DECLARE_OOXMLEXPORT_TEST(test_FieldType, "99_Fields.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Checking for three field types (BIBLIOGRAPHY, BIDIOUTLINE, CITATION) in sequence
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[2]/w:r[2]/w:instrText[1]",1);
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[5]/w:r[2]/w:instrText[1]",1);
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p/w:sdt/w:sdtContent/w:r[2]/w:instrText[1]",1);
}

DECLARE_OOXMLEXPORT_TEST(testCitation,"FDO74775.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:sdt/w:sdtContent/w:r[2]/w:instrText", " CITATION Kra06 \\l 1033 ");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:sdt/w:sdtContent/w:r[4]/w:t", "(Kramer & Chen, 2006)");
}

DECLARE_OOXMLEXPORT_TEST(testHyperLinkTagEnded, "fdo76316.docx")
{
    /* XML tag <w:hyperlink> was not getting closed when its inside another
     * <w:hyperlink> tag.
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc) return;

    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[2]/w:tc[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl[1]/w:tr[7]/w:tc[1]/w:tbl[1]/w:tr[2]/w:tc[6]/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[1]/w:hyperlink[1]/w:hyperlink[1]",1);
}

DECLARE_OOXMLEXPORT_TEST(testFDO76163 , "fdo76163.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    //docx file after RT is getting corrupted.
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[2]/w:hyperlink/w:r[11]/w:fldChar", "fldCharType", "end" );
}

DECLARE_OOXMLEXPORT_TEST(testFDO78659, "fdo78659.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[1]/w:hyperlink[1]/w:r[3]/w:fldChar[1]", 0);
}

DECLARE_OOXMLEXPORT_TEST(testFDO78654 , "fdo78654.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // In case of two "Hyperlink" tags in one paragraph and one of them
    // contains "PAGEREF" field then field end tag was missing from hyperlink.
    assertXPath ( pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[2]/w:hyperlink[3]/w:r[5]/w:fldChar", "fldCharType", "end" );
}


DECLARE_OOXMLEXPORT_TEST(testfdo78599,"fdo78599.docx")
{
     xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    //docx file after RT is getting corrupted.
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink/w:r[6]/w:fldChar", "fldCharType", "end" );

    // Check for automatic hyphenation
    if (xmlDocPtr pSettingsXml = parseExport("word/settings.xml"))
        // This failed as w:settings had no w:autoHyphenation child.
        assertXPath(pSettingsXml, "/w:settings/w:autoHyphenation");
}

DECLARE_OOXMLEXPORT_TEST(testfdo78886, "fdo78886.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[2]/w:tr[1]/w:tc[1]/w:p[1]/w:hyperlink[1]/w:r[2]/w:fldChar[1]", 0);
}

DECLARE_OOXMLEXPORT_TEST(testFdo78910, "fdo78910.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    // This is to ensure that the fld starts and ends inside a hyperlink...
    assertXPath ( pXmlDoc, "//w:hyperlink[2]/w:r[1]/w:fldChar", "fldCharType", "begin" );
    assertXPath ( pXmlDoc, "//w:hyperlink[2]/w:r[5]/w:fldChar", "fldCharType", "end" );
}

DECLARE_OOXMLEXPORT_TEST(testSdtCitationRun, "sdt-citation-run.docx")
{
    // The problem was that the SDT was around the whole paragraph, not only around the citation field.
    if (xmlDocPtr pXmlDoc = parseExport())
    {
        assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:t", "Before sdt.");
        assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r/w:instrText", " CITATION BBC11 \\l 1033 ");
        assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:t", "After sdt.");
    }
}

DECLARE_OOXMLEXPORT_TEST(testParagraphSdt, "paragraph-sdt.docx")
{
    // The problem was that the SDT was around the run only, not the whole paragraph.
    if (xmlDocPtr pXmlDoc = parseExport())
    {
        // The path to w:sdt contained a w:p.
        assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:sdt");
    }
}

DECLARE_OOXMLEXPORT_TEST(testSdt2Run, "sdt-2-para.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
    {
        // The problem was that <w:sdt> was closed after "first", not after "second", so the second assert failed.
        assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[1]/w:r/w:t", "first");
        assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[2]/w:r/w:t", "second");
        // Make sure the third paragraph is still outside <w:sdt>.
        assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r/w:t", "third");
    }
}

DECLARE_OOXMLEXPORT_TEST(test2Id, "2-id.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
    {
        // This was 2, but only one w:id is allowed.
        assertXPath(pXmlDoc, "//w:sdtPr/w:id", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testTableStart2Sdt, "table-start-2-sdt.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
    {
        // w:docPartGallery should be a child of <w:docPartObj>, make sure it's not a child of w:text.
        assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:text/w:docPartGallery", 0);
    }
}

DECLARE_OOXMLEXPORT_TEST(testSdtDateDuplicate, "sdt-date-duplicate.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
    {
        // Single <w:sdt> was exported as 2 <w:sdt> elements.
        assertXPath(pXmlDoc, "//w:sdt", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testFdo81492, "fdo81492.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
        assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[5]/w:instrText", "ADDIN EN.CITE.DATA");
}

DECLARE_OOXMLEXPORT_TEST(testEditTime, "fdo81341.docx")
{
    /* Issue was LO was not able to Import and Export EditTime in seconds format.
     * It was supporting Time in "HH:MM" format. But if DOCX conatins Time in seconds,
     * then LO was not able to display time in "HH:MM:SS" format.
     * While exporting LO was writing plain text instead of field entry.
     */
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        //Ensure that EditTime is written inside w:fldChar in "HH:MM:SS" format.
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:fldChar", "fldCharType", "begin");
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[3]/w:fldChar", "fldCharType", "separate");
        assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:t", "00:05");
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:fldChar", "fldCharType", "end");
    }
}

DECLARE_OOXMLEXPORT_TEST(testFdo81945, "fdo81945.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "//w:sdt//w:sdt", 0);
}

DECLARE_OOXMLEXPORT_TEST(testfdo82123, "fdo82123.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return;

    // make sure there is only one run inside first SDT after RT as in the Original file.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[2]/w:p/w:sdt[1]/w:sdtContent/w:r",1);
}

DECLARE_OOXMLEXPORT_TEST(testSdtBeforeField, "sdt-before-field.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // Make sure the field doesn't sneak inside the SDT: the SDT should contain only a single run (there were 6 ones).
        assertXPath(pXmlDoc, "//w:sdt/w:sdtContent/w:r", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testfdo81946, "fdo81946.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
       return;
    // make sure AlternateContent should not present in sdt
    assertXPath(pXmlDoc, "/w:hdr[1]/w:p[1]/w:sdt[1]/w:sdtContent[1]/w:r[2]/mc:AlternateContent[1]",0);
}

DECLARE_OOXMLEXPORT_TEST(testfdo82492, "fdo82492.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return;

    // make sure there is only one run inside first SDT after RT as in the Original file.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt[1]/w:sdtContent/w:r",1);
}

DECLARE_OOXMLEXPORT_TEST(testSdtHeader, "sdt-header.docx")
{
    // Problem was that w:sdt elements in headers were lost on import.
    if (xmlDocPtr pXmlDoc = parseExport("word/header1.xml"))
        // This was 0, w:sdt (and then w:date) was missing.
        assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:date", 1);
}

DECLARE_OOXMLEXPORT_TEST(testSdtCompanyMultipara, "sdt-company-multipara.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // This was 3, but multiple paragraphs inside "Company" SDT is now allowed.
        assertXPath(pXmlDoc, "//w:sdtContent/w:p", 1);
    }
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
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)24, date.Day);
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)7, date.Month);
    CPPUNIT_ASSERT_EQUAL((sal_Int16)2014, date.Year);

    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // Previously, fixed fields were exported as static text ("Date (fixed)")
        // Check they are now exported correctly as fldChar with fldLock attribute
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:fldChar", "fldLock", "true");
    }
}

DECLARE_OOXMLEXPORT_TEST(testOO34469, "ooo34469-1.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport())
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink[1]", "anchor", "2.9.2.Creating New files|outline");
}

DECLARE_OOXMLEXPORT_TEST(testOO39845, "ooo39845-7.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport())
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink[1]", "anchor", "Figure4|graphic");
}


CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
