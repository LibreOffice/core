/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf126245)
{
    createSwDoc("tdf126245.docx");

    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // export change tracking rejection data for tracked numbering change
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pPrChange/w:pPr/w:numPr/w:numId", "val",
                u"1");
}

CPPUNIT_TEST_FIXTURE(Test, testwDateValueFormat)
{
    createSwDoc("wDateValueFormat.docx");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 44
    // - validation error in OOXML export: Errors: 44
    saveAndReload(TestFilter::DOCX);

    // tdf#170457: round-tripped document was indicated as corrupt by MS Word
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    xmlDocUniquePtr pXmlComments = parseExport(u"word/comments.xml"_ustr);

    int nComments = countXPathNodes(pXmlComments, "//w:comment");

    int nCommentReferences = countXPathNodes(pXmlDoc, "//w:commentReference");
    // Each comment is referenced - the counts must match
    CPPUNIT_ASSERT_EQUAL(nComments, nCommentReferences);
    CPPUNIT_ASSERT_EQUAL(int(22), nCommentReferences);

    int nCommentStarts = countXPathNodes(pXmlDoc, "//w:commentRangeStart");
    int nCommentEnds = countXPathNodes(pXmlDoc, "//w:commentRangeEnd");
    CPPUNIT_ASSERT_EQUAL(nCommentStarts, nCommentEnds);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf124491)
{
    createSwDoc("tdf124491.docx");

    //FIXME: validation error in OOXML export: Errors: 5
    skipValidation();

    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import format change of empty lines, FIXME: change w:r with w:pPr in export
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/*/w:rPr/w:rPrChange");
    // empty line without format change
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/*/w:rPrChange", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/*/*/w:rPrChange", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143911)
{
    createSwDoc("tdf126206.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // export format change of text portions
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w:rPrChange");
    // This was without tracked bold formatting
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w:rPrChange/w:rPr/w:b");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105485)
{
    createSwDoc("tdf105485.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import change tracking of deleted comments
    assertXPath(pXmlDoc, "//w:del/w:r/w:commentReference");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125894)
{
    createSwDoc("tdf125894.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import change tracking in frames
    assertXPath(pXmlDoc, "//w:del", 2);
    assertXPath(pXmlDoc, "//w:ins");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149388)
{
    // see also testTdf132371
    createSwDoc("tdf132271.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import change tracking in floating tables
    // (don't recognize tracked text moving during the import,
    // because the text is too short and it's only a single word)
    assertXPath(pXmlDoc, "//w:del", 2);
    assertXPath(pXmlDoc, "//w:ins", 2);
    assertXPath(pXmlDoc, "//w:moveFrom", 0);
    assertXPath(pXmlDoc, "//w:moveTo", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132271)
{
    // see also testTdf149388
    createSwDoc("tdf149388.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import change tracking in floating tables
    assertXPath(pXmlDoc, "//w:del", 2);
    assertXPath(pXmlDoc, "//w:ins", 2);
    assertXPath(pXmlDoc, "//w:moveFrom", 0);
    assertXPath(pXmlDoc, "//w:moveTo", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149388_fly)
{
    // see also testTdf136667
    createSwDoc("tdf136667.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import change tracking in floating tables
    assertXPath(pXmlDoc, "//w:del", 2);
    assertXPath(pXmlDoc, "//w:ins", 4);
    assertXPath(pXmlDoc, "//w:moveFrom", 0);
    assertXPath(pXmlDoc, "//w:moveTo", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136667)
{
    // see also testTdf149388_fly
    createSwDoc("tdf149388_fly.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import change tracking in floating tables
    assertXPath(pXmlDoc, "//w:del", 2);
    assertXPath(pXmlDoc, "//w:ins", 4);
    assertXPath(pXmlDoc, "//w:moveFrom", 0);
    assertXPath(pXmlDoc, "//w:moveTo", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136850)
{
    createSwDoc("tdf136850.docx");

    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import change tracking in floating tables
    assertXPath(pXmlDoc, "//w:del");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128156)
{
    createSwDoc("tdf128156.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // keep tracked insertion of a paragraph
    // This was 0 before 350972a8bffc1a74b531e0336954bf54b1356025,
    // and 1 later (missing tracked insertion of the paragraph mark)
    assertXPath(pXmlDoc, "//w:ins", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165330)
{
    createSwDoc("CT-with-frame.docx");
    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // check that *both* tracked changes were round tripped
    assertXPathContent(pXmlDoc, "//w:p[1]/w:ins/w:r/w:t", u"world! ");
    assertXPathContent(pXmlDoc, "//w:p[3]/w:ins/w:r/w:t", u"hello ");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125546)
{
    createSwDoc("tdf125546.docx");

    //FIXME: validation error in OOXML export: Errors: 5
    skipValidation();

    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // compress redlines (it was 15)
    assertXPath(pXmlDoc, "//w:rPrChange", 3);
}

CPPUNIT_TEST_FIXTURE(Test, testLabelWidthAndPosition_Left_FirstLineIndent)
{
    createSwDoc("Hau_min_list2.fodt");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // list is LABEL_WIDTH_AND_POSITION with SvxAdjust::Left
    // I) LTR
    // a) all LTR cases with no number text look good in Word
    // 1) negative first line indent on paragraph:
    // no list width/indent: this one was 0 previously; this looks good
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "hanging", u"399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "end", u"0");
    // list width:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:ind", "hanging", u"966");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:ind", "end", u"0");
    // list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:ind", "hanging", u"399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:ind", "end", u"0");
    // list width + list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind", "start", u"1134");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind", "hanging", u"966");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind", "end", u"0");
    // 2) positive first line indent on paragraph:
    // no list width/indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind", "firstLine", u"420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind", "end", u"0");
    // list width:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:pPr/w:ind", "hanging", u"147");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:pPr/w:ind", "end", u"0");
    // list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:ind", "firstLine", u"420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:ind", "end", u"0");
    // list width + list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:pPr/w:ind", "start", u"1134");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:pPr/w:ind", "hanging", u"147");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:pPr/w:ind", "end", u"0");
    // b) all LTR cases with number text: the indent looks good but some tabs are wrong
    // 1) negative first line indent on paragraph:
    // no list width/indent: this one was 0 previously; this looks good
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:ind", "hanging", u"399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:ind", "end", u"0");
    // list width:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:pPr/w:ind", "hanging", u"966");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:pPr/w:ind", "end", u"0");
    // list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:ind", "hanging", u"399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:ind", "end", u"0");
    // list width + list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[12]/w:pPr/w:ind", "start", u"1134");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[12]/w:pPr/w:ind", "hanging", u"966");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[12]/w:pPr/w:ind", "end", u"0");
    // 2) positive first line indent on paragraph:
    // no list width/indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:ind", "firstLine", u"420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:ind", "end", u"0");
    // list width:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[14]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[14]/w:pPr/w:ind", "hanging", u"147");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[14]/w:pPr/w:ind", "end", u"0");
    // list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:ind", "firstLine", u"420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:ind", "end", u"0");
    // list width + list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[16]/w:pPr/w:ind", "start", u"1134");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[16]/w:pPr/w:ind", "hanging", u"147");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[16]/w:pPr/w:ind", "end", u"0");
    // (w:p[17] is empty)
    // I) RTL
    // a) only RTL cases with no number text and no width/indent look good in Word
    // 1) negative first line indent on paragraph:
    // no list width/indent
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[18]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[18]/w:pPr/w:ind", "hanging", u"399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[18]/w:pPr/w:ind", "end", u"0");
    // 2) positive first line indent on paragraph:
    // no list width/indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[22]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[22]/w:pPr/w:ind", "firstLine", u"420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[22]/w:pPr/w:ind", "end", u"0");
    // b) RTL cases with number text: the indent looks good but some tabs are wrong
    // 1) negative first line indent on paragraph:
    // no list width/indent
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[26]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[26]/w:pPr/w:ind", "hanging", u"399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[26]/w:pPr/w:ind", "end", u"0");
    // 2) positive first line indent on paragraph:
    // no list width/indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[30]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[30]/w:pPr/w:ind", "firstLine", u"420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[30]/w:pPr/w:ind", "end", u"0");
    // TODO: other cases
}

CPPUNIT_TEST_FIXTURE(Test, testTdf124604)
{
    createSwDoc("tdf124604.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // If the numbering comes from a base style, indentation of the base style has also priority.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:ind", "start", u"0");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf95374)
{
    createSwDoc("tdf95374.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Numbering disabled by non-existent numId=0, disabling also inheritance of indentation of parent styles
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "hanging", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "start", u"1136");
}

DECLARE_OOXMLEXPORT_TEST(testTdf108493, "tdf108493.docx")
{
    uno::Reference<beans::XPropertySet> xPara7(getParagraph(7), uno::UNO_QUERY);
    // set in the paragraph (709 twips)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1251), getProperty<sal_Int32>(xPara7, u"ParaLeftMargin"_ustr));
    // set in the numbering style (this was 0)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1251),
                         getProperty<sal_Int32>(xPara7, u"ParaFirstLineIndent"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf118691, "tdf118691.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // Text "Before" stays in the first cell, not removed before the table because of
    // bad handling of <w:cr>
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Before\nAfter"_ustr, xCell->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf64264, "tdf64264.docx")
{
    // DOCX table rows with tblHeader setting mustn't modify the count of the
    // repeated table header rows, when there is rows before them without tblHeader settings.
    xmlDocUniquePtr pDump = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // table starts on page 1 and finished on page 2
    // and it has got only a single repeating header line
    assertXPath(pDump, "/root/page[2]/body/tab", 1);
    assertXPath(pDump, "/root/page[2]/body/tab/row", 47);
    assertXPathContent(pDump, "/root/page[2]/body/tab/row[1]/cell[1]/txt/text()",
                       u"Repeating Table Header");
    assertXPathContent(pDump, "/root/page[2]/body/tab/row[2]/cell[1]/txt/text()", u"Text");
}

DECLARE_OOXMLEXPORT_TEST(testTdf58944RepeatingTableHeader, "tdf58944-repeating-table-header.docx")
{
    // DOCX tables with more than 10 repeating header lines imported without repeating header lines
    // as a workaround for MSO's limitation of header line repetition
    xmlDocUniquePtr pDump = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // table starts on page 1 and finished on page 2
    // instead of showing only a part of it on page 2
    assertXPath(pDump, "/root/page[1]/body/tab", 1);
    assertXPath(pDump, "/root/page[1]/body/tab/row", 11);
    assertXPathContent(pDump, "/root/page[2]/body/tab/row[1]/cell[1]/txt/text()", u"Test1");
    assertXPathContent(pDump, "/root/page[2]/body/tab/row[2]/cell[1]/txt/text()", u"Test2");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf81100)
{
    auto verify = [this](bool bIsExport = false) {
        xmlDocUniquePtr pDump = parseLayoutDump();
        CPPUNIT_ASSERT_EQUAL(3, getPages());

        // table starts on page 1 and finished on page 2
        // and it has got only a single repeating header line
        assertXPath(pDump, "/root/page[2]/body/tab[1]", 1);
        assertXPath(pDump, "/root/page[2]/body/tab[1]/row", 2);
        assertXPath(pDump, "/root/page[3]/body/tab", 1);
        if (!bIsExport) // TODO export tblHeader=false
            assertXPath(pDump, "/root/page[3]/body/tab/row", 1);
    };
    createSwDoc("tdf81100.docx");
    verify();

    saveAndReload(TestFilter::DOCX);
    verify(/*bIsExport*/ true);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // keep "repeat table header" setting of table styles
    assertXPath(pXmlDoc, "/w:styles/w:style/w:tblStylePr/w:trPr/w:tblHeader", 4);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf88496)
{
    createSwDoc("tdf88496.docx");
    saveAndReload(TestFilter::DOCX);
    // Switch off repeating header, there is no place for it.
    // Now there are only 3 pages with complete table content
    // instead of a 51-page long table only with header.
    CPPUNIT_ASSERT_EQUAL(3, getPages());
    // (this appears to have the correct result now?)
    // FIXME: this actually has 3 pages but SwWrtShell::SttPg() puts the cursor
    // into the single SwTextFrame in the follow-flow-row at the top of the
    // table but that SwTextFrame 1105 should not exist and the cursor ends up
    // at the end of its master frame 848 instead; the problem is somewhere in
    // SwTextFrame::FormatAdjust() which first determines nNew = 1 but then
    // grows this frame anyway so that the follow is empty, but nothing
    // invalidates 1105 again.
}

CPPUNIT_TEST_FIXTURE(Test, testTdf77417)
{
    createSwDoc("tdf77417.docx");
    saveAndReload(TestFilter::DOCX);
    // MSO 2010 compatibility mode: terminating white spaces are ignored in tables.
    // This was 3 pages with the first invisible blank page.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130494)
{
    createSwDoc("tdf130494.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:pPr/w:rPr/w:highlight", "val",
                u"yellow");
    // keep direct formatting of table cell paragraph with removed highlighting
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r/w:rPr/w:highlight", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130690)
{
    createSwDoc("tdf130690.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:pPr/w:rPr/w:highlight", "val",
                u"yellow");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[1]/w:rPr/w:highlight", 1);
    // keep direct formatting of table cell paragraph with removed highlighting
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[2]/w:rPr/w:highlight", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105215)
{
    createSwDoc("tdf105215.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:pPr/w:rPr/w:rFonts", "ascii",
                u"Linux Libertine G");

    // These were "Linux Libertine G"
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r/w:rPr", 5);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[1]/w:rPr/w:rFonts", "ascii",
                u"Lohit Devanagari");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[2]/w:rPr/w:rFonts", "ascii",
                u"Lohit Devanagari");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[3]/w:rPr/w:rFonts", "ascii",
                u"Lohit Devanagari");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[4]/w:rPr/w:rFonts", "ascii",
                u"Lohit Devanagari");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[5]/w:rPr/w:rFonts", "ascii",
                u"Lohit Devanagari");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf135187)
{
    createSwDoc("tdf135187.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:p/w:pPr/w:rPr/w:b", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[1]/w:p/w:pPr/w:rPr/w:b", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[1]/w:p/w:pPr/w:rPr/w:b", 1);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b", 1);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b",
                           "val");
    // This was 0
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b", 1);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b",
                           "val");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b", "val",
                u"false");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136617)
{
    createSwDoc("tdf136617.docx");
    save(TestFilter::DOCX);

    // This was 2
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:p[2]/w:pPr/w:rPr/w:sz", "val",
                u"16");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121597TrackedDeletionOfMultipleParagraphs)
{
    createSwDoc("tdf121597.odt");
    save(TestFilter::DOCX);
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // check paragraphs with removed paragraph mark
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:pPr/w:rPr/w:del");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf141660)
{
    createSwDoc("tdf141660.docx");

    save(TestFilter::DOCX);
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:footnoteReference", "id", u"2");
    // w:del is imported correctly with its footnote
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:del[2]/w:r/w:footnoteReference", "id", u"3");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/w:footnoteReference", "id", u"4");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133643)
{
    createSwDoc("tdf133643.doc");
    save(TestFilter::DOCX);
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[1]/w:fldChar", "fldCharType",
                u"begin");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[1]/w:fldChar/w:ffData/w:ddList/w:listEntry[1]",
        "val", u"Bourgoin-Jallieu, ");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[1]/w:fldChar/w:ffData/w:ddList/w:listEntry[2]",
        "val", u"Fontaine, ");

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[2]/w:instrText",
                       u" FORMDROPDOWN ");

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[3]/w:fldChar", "fldCharType",
                u"separate");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[5]/w:fldChar", "fldCharType",
                u"end");

    // Without the fix in place, this w:r wouldn't exist
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[6]/w:t",
                       u"le 22 fevrier 2013");
}

DECLARE_OOXMLEXPORT_TEST(testTdf123189_tableBackground, "table-black_fill.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    uno::Reference<table::XCell> xCell = xTable->getCellByName(u"A1"_ustr);
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, getProperty<Color>(xCell, u"BackColor"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf116084, "tdf116084.docx")
{
    // tracked line is not a single text portion: w:del is recognized within w:ins
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(1), 1)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 1), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"There "_ustr, getRun(getParagraph(1), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(1), 4)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 4), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"must"_ustr, getRun(getParagraph(1), 5)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf116084_anonymized)
{
    createSwDoc("tdf116084.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // w:del in w:ins is exported correctly
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del/w:r/w:delText", u"must");

    // no date (anonymized changes)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins[@date]", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del[@w:date]", 0);

    // w:ins and w:del have w:author attributes, and the same
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del[@w:author]", 1);
    OUString sAuthor = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins[2]", "author");
    OUString sAuthor2 = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del", "author");
    CPPUNIT_ASSERT_EQUAL(sAuthor, sAuthor2);
}

DECLARE_OOXMLEXPORT_TEST(testTdf121176, "tdf121176.docx")
{
    // w:del is imported correctly when it is in a same size w:ins
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(1), 1)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 1), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"must"_ustr, getRun(getParagraph(1), 2)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121176_anonymized)
{
    createSwDoc("tdf121176.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // w:del in w:ins is exported correctly
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del/w:r/w:delText", u"must");

    // no date (anonymized changes)
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:p/w:ins", "date");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del[@w:date]", 0);

    // w:ins and w:del have w:author attributes, and the same
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del[@w:author]", 1);
    OUString sAuthor = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins", "author");
    OUString sAuthor2 = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del", "author");
    CPPUNIT_ASSERT_EQUAL(sAuthor, sAuthor2);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128913)
{
    createSwDoc("tdf128913.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // w:ins and w:del are imported correctly, if they contain only inline images
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:r/w:drawing/wp:inline/a:graphic");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:del/w:r/w:drawing/wp:inline/a:graphic");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf142700)
{
    createSwDoc("tdf142700.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // w:ins and w:del are imported correctly, if they contain only images anchored to character
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:r/w:drawing/wp:anchor/a:graphic");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:del/w:r/w:drawing/wp:anchor/a:graphic");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf142387)
{
    createSwDoc("tdf142387.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // w:del in w:ins is exported correctly (only w:del was exported)
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del/w:r/w:delText", u"inserts ");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147892)
{
    createSwDoc("tdf147892.fodt");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // w:del in w:ins is exported correctly
    // (both w:del and w:ins were exported for para marker)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:del", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:ins", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf123054, "tdf123054.docx")
{
    //FIXME: validation error in OOXML export: Errors: 64
    skipValidation();

    CPPUNIT_ASSERT_EQUAL(u"No Spacing"_ustr,
                         getProperty<OUString>(getParagraph(20), u"ParaStyleName"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf67207_MERGEFIELD_DATABASE, "tdf67207.docx")
{
    // database fields use the database "database" and its table "Sheet1"
    uno::Reference<beans::XPropertySet> xTextField
        = getProperty<uno::Reference<beans::XPropertySet>>(getRun(getParagraph(2), 2),
                                                           u"TextField"_ustr);
    CPPUNIT_ASSERT(xTextField.is());
    uno::Reference<lang::XServiceInfo> xServiceInfo(xTextField, uno::UNO_QUERY_THROW);
    uno::Reference<text::XDependentTextField> xDependent(xTextField, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.TextField.Database"_ustr));
    OUString sValue;
    xTextField->getPropertyValue(u"Content"_ustr) >>= sValue;
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8("<c1>"), sValue);

    uno::Reference<beans::XPropertySet> xFiledMaster = xDependent->getTextFieldMaster();
    uno::Reference<lang::XServiceInfo> xFiledMasterServiceInfo(xFiledMaster, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(
        xFiledMasterServiceInfo->supportsService(u"com.sun.star.text.fieldmaster.Database"_ustr));

    // Defined properties: DataBaseName, Name, DataTableName, DataColumnName, DependentTextFields, DataCommandType, InstanceName, DataBaseURL
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue(u"DataBaseName"_ustr) >>= sValue);
    CPPUNIT_ASSERT_EQUAL(u"database"_ustr, sValue);
    sal_Int32 nCommandType;
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue(u"DataCommandType"_ustr) >>= nCommandType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nCommandType); // css::sdb::CommandType::TABLE
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue(u"DataTableName"_ustr) >>= sValue);
    CPPUNIT_ASSERT_EQUAL(u"Sheet1"_ustr, sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue(u"DataColumnName"_ustr) >>= sValue);
    CPPUNIT_ASSERT_EQUAL(u"c1"_ustr, sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue(u"InstanceName"_ustr) >>= sValue);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.text.fieldmaster.DataBase.database.Sheet1.c1"_ustr, sValue);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf101122_noFillForCustomShape)
{
    createSwDoc("tdf101122_noFillForCustomShape.odt");
    save(TestFilter::DOCX);
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // tdf#101122 check whether the "F" (noFill) option has been exported to docx
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:custGeom/a:pathLst/a:path",
                "fill", u"none");
    assertXPathNoAttribute(
        pXmlDoc,
        "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
        "a:graphicData/wps:wsp/wps:spPr/a:custGeom/a:pathLst/a:path",
        "fill");
}
// The (tdf124678_no_leading_paragraph.odt, tdf124678_with_leading_paragraph.odt) documents are the same,
// except:
// - tdf124678_no_leading_paragraph.odt doesn't contain leading empty paragraph
//   before the first section
//
CPPUNIT_TEST_FIXTURE(Test, testTdf124678_case1)
{
    createSwDoc("tdf124678_no_leading_paragraph.odt");
    saveAndReload(TestFilter::DOCX);
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("First page header text", u""_ustr,
                                 getXPathContent(pXmlDoc, "/root/page[1]/header/txt"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second page header text", u"HEADER"_ustr,
                                 getXPathContent(pXmlDoc, "/root/page[2]/header/txt"));
}

// The (tdf124678_no_leading_paragraph.odt, tdf124678_with_leading_paragraph.odt) documents are the same,
// except:
// - tdf124678_no_leading_paragraph.odt doesn't contain leading empty paragraph
//   before the first section
//
CPPUNIT_TEST_FIXTURE(Test, testTdf124678_case2)
{
    createSwDoc("tdf124678_with_leading_paragraph.odt");
    saveAndReload(TestFilter::DOCX);
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("First page header text", u""_ustr,
                                 getXPathContent(pXmlDoc, "/root/page[1]/header/txt"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second page header text", u"HEADER"_ustr,
                                 getXPathContent(pXmlDoc, "/root/page[2]/header/txt"));
}

static bool lcl_nearEqual(const sal_Int32 nNumber1, const sal_Int32 nNumber2,
                          sal_Int32 nMaxDiff = 5)
{
    return std::abs(nNumber1 - nNumber2) < nMaxDiff;
}

CPPUNIT_TEST_FIXTURE(Test, testTdf119952_negativeMargins)
{
    auto verify = [this](bool bIsExport = false) {
        // With negative margins (in MS Word) one can set up header (or footer) that overlaps with the body.
        // LibreOffice unable to display that, so when importing negative margins,
        // the header (or footer) converted to a flyframe, anchored to the header..
        // that can overlap with the body, and will appear like in Word.
        // This conversion modifies the document [i.e. replacing header text with a textbox...]
        // but its DOCX export looks the same, as the original document in Word, too.
        xmlDocUniquePtr pDump = parseLayoutDump();

        //Check layout positions / sizes
        sal_Int32 nLeftHead = getXPath(pDump, "//page[1]/header/infos/bounds", "left").toInt32();
        sal_Int32 nLeftBody = getXPath(pDump, "//page[1]/body/infos/bounds", "left").toInt32();
        sal_Int32 nLeftFoot = getXPath(pDump, "//page[1]/footer/infos/bounds", "left").toInt32();
        sal_Int32 nLeftHFly
            = getXPath(pDump, "//page[1]/header/txt/anchored/fly/infos/bounds", "left").toInt32();
        sal_Int32 nLeftFFly
            = getXPath(pDump, "//page[1]/footer/txt/anchored/fly/infos/bounds", "left").toInt32();

        sal_Int32 nTopHead = getXPath(pDump, "//page[1]/header/infos/bounds", "top").toInt32();
        sal_Int32 nTopBody = getXPath(pDump, "//page[1]/body/infos/bounds", "top").toInt32();
        sal_Int32 nTopFoot = getXPath(pDump, "//page[1]/footer/infos/bounds", "top").toInt32();
        sal_Int32 nTopHFly
            = getXPath(pDump, "//page[1]/header/txt/anchored/fly/infos/bounds", "top").toInt32();
        sal_Int32 nTopFFly
            = getXPath(pDump, "//page[1]/footer/txt/anchored/fly/infos/bounds", "top").toInt32();

        sal_Int32 nHeightHead
            = getXPath(pDump, "//page[1]/header/infos/bounds", "height").toInt32();
        sal_Int32 nHeightBody = getXPath(pDump, "//page[1]/body/infos/bounds", "height").toInt32();
        sal_Int32 nHeightFoot
            = getXPath(pDump, "//page[1]/footer/infos/bounds", "height").toInt32();
        sal_Int32 nHeightHFly
            = getXPath(pDump, "//page[1]/header/txt/anchored/fly/infos/bounds", "height").toInt32();
        sal_Int32 nHeightFFly
            = getXPath(pDump, "//page[1]/footer/txt/anchored/fly/infos/bounds", "height").toInt32();
        sal_Int32 nHeightHFlyBound
            = getXPath(pDump, "//page[1]/header/infos/prtBounds", "height").toInt32();
        sal_Int32 nHeightFFlyBound
            = getXPath(pDump, "//page[1]/footer/infos/prtBounds", "height").toInt32();

        CPPUNIT_ASSERT(lcl_nearEqual(nLeftHead, nLeftBody));
        CPPUNIT_ASSERT(lcl_nearEqual(nLeftHead, nLeftFoot));
        CPPUNIT_ASSERT(lcl_nearEqual(nLeftHead, nLeftHFly));
        CPPUNIT_ASSERT(lcl_nearEqual(nLeftHead, nLeftFFly));

        CPPUNIT_ASSERT(lcl_nearEqual(nTopHead, 851));
        CPPUNIT_ASSERT(lcl_nearEqual(nTopBody, 1418));
        CPPUNIT_ASSERT(lcl_nearEqual(nTopFoot, 15875));
        CPPUNIT_ASSERT(lcl_nearEqual(nTopHFly, 851));

        // this seems to be an import bug
        if (!bIsExport)
            CPPUNIT_ASSERT(lcl_nearEqual(nTopFFly, 14403));

        CPPUNIT_ASSERT(lcl_nearEqual(nHeightHead, 567));
        CPPUNIT_ASSERT(lcl_nearEqual(nHeightBody, 14457));
        CPPUNIT_ASSERT(lcl_nearEqual(nHeightFoot, 680));
        CPPUNIT_ASSERT(lcl_nearEqual(nHeightHFly, 2152));
        CPPUNIT_ASSERT(lcl_nearEqual(nHeightFFly, 2152));

        // after export these heights increase to like 567.
        // not sure if it is another import, or export bug... or just the result of the modified document
        if (!bIsExport)
        {
            CPPUNIT_ASSERT(lcl_nearEqual(nHeightHFlyBound, 57));
            CPPUNIT_ASSERT(lcl_nearEqual(nHeightFFlyBound, 57));
        }

        //Check text of header/ footer
        assertXPath(
            pDump,
            "//page[1]/header/txt/anchored/fly/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"f1");
        assertXPath(
            pDump,
            "//page[1]/header/txt/anchored/fly/txt[8]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"                f8");
        assertXPath(
            pDump,
            "//page[1]/footer/txt/anchored/fly/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"                f8");
        assertXPath(
            pDump,
            "//page[1]/footer/txt/anchored/fly/txt[8]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"f1");

        assertXPath(
            pDump,
            "//page[2]/header/txt/anchored/fly/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"p1");
        assertXPath(
            pDump,
            "//page[2]/footer/txt/anchored/fly/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"p1");

        assertXPath(
            pDump,
            "//page[3]/header/txt/anchored/fly/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"  aaaa");
        assertXPath(
            pDump,
            "//page[3]/header/txt/anchored/fly/txt[5]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"      eeee");

        assertXPathContent(pDump, "/root/page[1]/header/txt/anchored/fly",
                           u"f1    f2      f3        f4          f5            f6            "
                           "  f7                f8");
        assertXPathContent(pDump, "/root/page[1]/footer/txt/anchored/fly",
                           u"                f8              f7            f6          f5    "
                           "    f4      f3    f2f1");
        assertXPathContent(pDump, "/root/page[2]/header/txt/anchored/fly", u"p1");
        assertXPathContent(pDump, "/root/page[2]/footer/txt/anchored/fly", u"p1");
        assertXPathContent(pDump, "/root/page[3]/header/txt/anchored/fly",
                           u"  aaaa   bbbb    cccc     dddd      eeee");
    };

    createSwDoc("tdf119952_negativeMargins.docx");
    verify();
    saveAndReload(TestFilter::DOCX);
    verify(/*bIsExport*/ true);
}

DECLARE_OOXMLEXPORT_TEST(testTdf143384_tableInFoot_negativeMargins,
                         "tdf143384_tableInFoot_negativeMargins.docx")
{
    // There should be no crash during loading of the document
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
