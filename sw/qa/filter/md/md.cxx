/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <swmodeltestbase.hxx>

#include <com/sun/star/style/ParagraphAdjust.hpp>

#include <comphelper/propertyvalue.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>

#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <poolfmt.hxx>
#include <charatr.hxx>
#include <fmtanchr.hxx>
#include <IDocumentContentOperations.hxx>
#include <fmtcntnt.hxx>
#include <ndgrf.hxx>
#include <itabenum.hxx>
#include <ndtxt.hxx>
#include <fmturl.hxx>
#include <textcontentcontrol.hxx>
#include <fmtfsize.hxx>

namespace
{
/**
 * Covers sw/source/filter/md/ fixes.
 *
 * Note that these tests are meant to be simple: either load a file and assert some result or build
 * a document model with code, export and assert that result.
 *
 * Keep using the various sw_<format>import/export suites for multiple filter calls inside a single
 * test.
 */

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/filter/md/data/"_ustr)
    {
    }

    std::string TempFileToString()
    {
        SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
        std::vector<char> aBuffer(aStream.remainingSize());
        aStream.ReadBytes(aBuffer.data(), aBuffer.size());
        return std::string(aBuffer.data(), aBuffer.size());
    }
};
}

CPPUNIT_TEST_FIXTURE(Test, testExportFormula)
{
    createSwDoc("tdf168572.odt");

    // Without the fix in place, this test would have crashed here
    save(TestFilter::MD);

    std::string aActual = TempFileToString();

    CPPUNIT_ASSERT(aActual.starts_with("![]("));
    CPPUNIT_ASSERT(aActual.ends_with(")" SAL_NEWLINE_STRING));
}

CPPUNIT_TEST_FIXTURE(Test, testExportTableFrame)
{
    createSwDoc("table.odt");

    // Without the fix in place, this test would have crashed here
    save(TestFilter::MD);

    std::string aActual = TempFileToString();
    std::string aExpected("Text" SAL_NEWLINE_STRING SAL_NEWLINE_STRING
                          "![]()Text" SAL_NEWLINE_STRING);

    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testExportingBasicElements)
{
    createSwDoc("basic-elements.fodt");

    save(TestFilter::MD);
    std::string aActual = TempFileToString();
    std::string aExpected(
        // clang-format off
        "# Heading 1" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "## Heading 2" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "### Heading 3" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "#### Heading 4" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "##### Heading 5" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "###### Heading 6" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "**Bold** text" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "Text in *italics*" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "This is a [hyperlink](http://www.libreoffice.org/)" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "\\# Leading hash" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "Some \\{braces\\}, \\[square brackets\\], \\*asterisks\\*, \\`backticks\\`, \\\\backslashes\\\\, \\_underscores\\_, \\<angle brackets\\>" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        // clang-format on
    );

    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf169884_import_crash)
{
    setImportFilterName(TestFilter::MD);
    createSwDoc("tdf169884.md");

    CPPUNIT_ASSERT_EQUAL(OUString("Selected range of Release "
                                  "note\nhttps://wiki.documentfoundation.org/ReleaseNotes/"
                                  "26.2#Markdown::text=Added%20support%20for%20importing%20from%"
                                  "20Markdown%20format"),
                         getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testHeading)
{
    setImportFilterName(TestFilter::MD);
    createSwDoc("heading.md");

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1),
                         getProperty<sal_Int16>(getParagraph(1), u"OutlineLevel"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testList)
{
    setImportFilterName(TestFilter::MD);
    createSwDoc("list.md");

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(SVX_NUM_ARABIC), getNumberingTypeOfParagraph(1));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0),
                         getProperty<sal_Int16>(getParagraph(1), u"NumberingLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(OUString("Ordered"), getParagraph(1)->getString());

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(SVX_NUM_CHAR_SPECIAL),
                         getNumberingTypeOfParagraph(2));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1),
                         getProperty<sal_Int16>(getParagraph(2), u"NumberingLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(OUString("Unordered"), getParagraph(2)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testExportingRedlines)
{
    // Given a document with some redlines
    createSwDoc("redlines-and-comments.odt");

    // Save as a markdown document
    save(TestFilter::MD);
    SvFileStream fileStream(maTempFile.GetURL(), StreamMode::READ);
    OUString aParagraph;
    // 1st paragraph
    CPPUNIT_ASSERT(fileStream.ReadUniOrByteStringLine(aParagraph, RTL_TEXTENCODING_UTF8));
    // Check that the insert/delete redlines were exported as <del>/<ins> elements
    std::u16string_view expected
        = uR"(<del title="Author: Author 1" datetime="2019-04-23T09:25:00"> </del>)";
    CPPUNIT_ASSERT(aParagraph.indexOf(expected) >= 0);
    expected = uR"(<ins title="Author: Author 1" datetime="2019-04-23T09:25:00">)";
    CPPUNIT_ASSERT(aParagraph.indexOf(expected) >= 0);
    // The insert starts on the first paragraph, and ends on the second
    CPPUNIT_ASSERT(aParagraph.indexOf("</ins>") < 0);
    // An empty line between paragraphs
    CPPUNIT_ASSERT(fileStream.ReadUniOrByteStringLine(aParagraph, RTL_TEXTENCODING_UTF8));
    CPPUNIT_ASSERT(aParagraph.isEmpty());
    // 2nd paragraph
    CPPUNIT_ASSERT(fileStream.ReadUniOrByteStringLine(aParagraph, RTL_TEXTENCODING_UTF8));
    CPPUNIT_ASSERT(aParagraph.indexOf("</ins>") >= 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTables)
{
    setImportFilterName(TestFilter::MD);
    createSwDoc("tables.md");

    uno::Reference<text::XTextContent> const xtable(getParagraphOrTable(1));

    // cell A1

    uno::Reference<text::XText> xCellText(getCell(xtable, u"A1"_ustr), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL(u"Left-aligned"_ustr, xCellText->getString());

    uno::Reference<text::XTextCursor> xCursor = xCellText->createTextCursor();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(style::ParagraphAdjust_LEFT),
                         getProperty<sal_Int16>(xCursor, u"ParaAdjust"_ustr));

    // cell B1
    xCellText.set(getCell(xtable, u"B1"_ustr), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL(u"Center-aligned"_ustr, xCellText->getString());

    xCursor = xCellText->createTextCursor();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(style::ParagraphAdjust_CENTER),
                         getProperty<sal_Int16>(xCursor, u"ParaAdjust"_ustr));

    // cell C1
    xCellText.set(getCell(xtable, u"C1"_ustr), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL(u"Right-aligned"_ustr, xCellText->getString());

    xCursor = xCellText->createTextCursor();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(style::ParagraphAdjust_RIGHT),
                         getProperty<sal_Int16>(xCursor, u"ParaAdjust"_ustr));

    // cell A2
    CPPUNIT_ASSERT(getCell(xtable, u"A2"_ustr, u"data1"_ustr).is());

    // cell B2
    xCellText.set(getCell(xtable, u"B2"_ustr), uno::UNO_QUERY_THROW);

    uno::Reference<text::XTextContent> xContent(getShape(1), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL(u"data2 "_ustr, xCellText->getString());
    CPPUNIT_ASSERT_EQUAL(xCellText, xContent->getAnchor()->getText());

    // cell C2
    CPPUNIT_ASSERT(getCell(xtable, u"C2"_ustr, u"data3"_ustr).is());
}

CPPUNIT_TEST_FIXTURE(Test, testExportingCodeSpan)
{
    // Given a document where the middle word is a code portion:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    IDocumentStylePoolAccess& rIDSPA = pDoc->getIDocumentStylePoolAccess();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert(u"A B C"_ustr);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    SwView& rView = pWrtShell->GetView();
    SwTextFormatColl* pColl = rIDSPA.GetTextCollFromPool(SwPoolFormatId::COLL_HTML_PRE);
    SfxItemSetFixed<RES_CHRATR_BEGIN, RES_CHRATR_END> aSet(rView.GetPool());
    aSet.Put(pColl->GetFont());
    pWrtShell->SetAttrSet(aSet);

    // When saving that to markdown:
    save(TestFilter::MD);

    // Then make sure the format of B is exported:
    std::string aActual = TempFileToString();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: A `B` C
    // - Actual  : A B C
    // i.e. the code formatting was lost.
    std::string aExpected("A `B` C" SAL_NEWLINE_STRING);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testExportingList)
{
    // Given a document that has both toplevel/nested bullets/numberings:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert(u"A"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"B"_ustr);
    pWrtShell->BulletOn();
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"C"_ustr);
    pWrtShell->NumUpDown(/*bDown=*/true);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"D"_ustr);
    pWrtShell->DelNumRules();
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"E"_ustr);
    pWrtShell->NumOn();
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"F"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"G"_ustr);
    pWrtShell->NumUpDown(/*bDown=*/true);

    // When saving that to markdown:
    save(TestFilter::MD);

    // Then make sure list type and level is exported:
    std::string aActual = TempFileToString();
    std::string aExpected(
        // clang-format off
        "A" SAL_NEWLINE_STRING SAL_NEWLINE_STRING
        "- B" SAL_NEWLINE_STRING SAL_NEWLINE_STRING
        // indent is 2 spaces
        "  - C" SAL_NEWLINE_STRING SAL_NEWLINE_STRING
        "D" SAL_NEWLINE_STRING SAL_NEWLINE_STRING
        "1. E" SAL_NEWLINE_STRING SAL_NEWLINE_STRING
        "2. F" SAL_NEWLINE_STRING SAL_NEWLINE_STRING
        // indent is 3 spaces
        "   1. G" SAL_NEWLINE_STRING
        // clang-format on
    );
    // Without the accompanying fix in place, this test would have failed, all the "- " and "1. "
    // style prefixes were lost.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testExportingImage)
{
    // Given a document with an inline, linked image:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert(u"A "_ustr);
    SfxItemSet aFrameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SwFormatAnchor aAnchor(RndStdIds::FLY_AS_CHAR);
    aFrameSet.Put(aAnchor);
    Graphic aGraphic;
    OUString aGraphicURL(u"./test.png"_ustr);
    IDocumentContentOperations& rIDCO = pDoc->getIDocumentContentOperations();
    SwCursor* pCursor = pWrtShell->GetCursor();
    SwFlyFrameFormat* pFlyFormat
        = rIDCO.InsertGraphic(*pCursor, aGraphicURL, OUString(), &aGraphic, &aFrameSet,
                              /*pGrfAttrSet=*/nullptr, /*SwFrameFormat=*/nullptr);
    pFlyFormat->SetObjDescription(u"mydesc"_ustr);
    pWrtShell->Insert(u" B"_ustr);

    // When saving that to markdown:
    save(TestFilter::MD);

    // Then make sure the image is exported:
    std::string aActual = TempFileToString();
    std::string aExpected("A ![mydesc](./test.png) B" SAL_NEWLINE_STRING);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: A ![mydesc](./test.png) B
    // - Actual  : A  B
    // i.e. the image was lost.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testExportingTable)
{
    // Given a document that has a table:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert(u"before"_ustr);
    SwInsertTableOptions aInsertTableOptions(SwInsertTableFlags::DefaultBorder,
                                             /*nRowsToRepeat=*/0);
    pWrtShell->InsertTable(aInsertTableOptions, /*nRows=*/3, /*nCols=*/3);
    pWrtShell->Insert(u"after"_ustr);
    pWrtShell->SttPara();
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    pWrtShell->Insert(u"A1"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"B1"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"C1"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"A2"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"B2"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"C2"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"A3"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"B3"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"C3"_ustr);

    // When saving that to markdown:
    save(TestFilter::MD);

    // Then make sure the table content is not lost:
    std::string aActual = TempFileToString();
    std::string aExpected(
        // clang-format off
        "before" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "| A1 | B1 | C1 |" SAL_NEWLINE_STRING
        // Delimiter row consists of cells whose only content are hyphens (-).
        "| - | - | - |" SAL_NEWLINE_STRING
        "| A2 | B2 | C2 |" SAL_NEWLINE_STRING
        "| A3 | B3 | C3 |" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "after" SAL_NEWLINE_STRING
        // clang-format on
    );
    // Without the accompanying fix in place, this test would have failed with:
    // - Actual  : before\n\nafter\n
    // i.e. the table content was lost.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testBlockQuoteMdImport)
{
    // Given a document with a "block quote" 2nd paragraph:
    // When importing that document:
    setImportFilterName(TestFilter::MD);
    createSwDoc("quote.md");

    // Then make sure that the paragraph style is set correctly:
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    SwCursor* pCursor = pWrtShell->GetCursor();
    SwTextNode* pTextNode = pCursor->GetPointNode().GetTextNode();
    SwFormatColl* pActual = pTextNode->GetFormatColl();
    SwDoc* pDoc = pDocShell->GetDoc();
    IDocumentStylePoolAccess& rIDSPA = pDoc->getIDocumentStylePoolAccess();
    SwFormatColl* pExpected = rIDSPA.GetTextCollFromPool(SwPoolFormatId::COLL_HTML_BLOCKQUOTE);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Block Quotation
    // - Actual  : Body Text
    // i.e. the paragraph style was not set.
    CPPUNIT_ASSERT_EQUAL(pExpected->GetName().toString(), pActual->GetName().toString());
}

CPPUNIT_TEST_FIXTURE(Test, testBlockQuoteMdExport)
{
    // Given a document where the only paragraph is a block quote:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwCursor* pCursor = pWrtShell->GetCursor();
    SwDoc* pDoc = pDocShell->GetDoc();
    IDocumentStylePoolAccess& rIDSPA = pDoc->getIDocumentStylePoolAccess();
    SwTextFormatColl* pColl = rIDSPA.GetTextCollFromPool(SwPoolFormatId::COLL_HTML_BLOCKQUOTE);
    pDoc->SetTextFormatColl(*pCursor, pColl);
    pWrtShell->Insert(u"test"_ustr);

    // When saving that to markdown:
    save(TestFilter::MD);

    // Then make sure the format of the paragraph is exported:
    std::string aActual = TempFileToString();
    std::string aExpected("> test" SAL_NEWLINE_STRING);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: > test
    // - Actual  : test
    // i.e. the block quote markup was missing.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testCodeBlockMdExport)
{
    // Given a document that has a multi-paragraph code block:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert(u"A"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"B"_ustr);
    SwCursor* pCursor = pWrtShell->GetCursor();
    SwDoc* pDoc = pDocShell->GetDoc();
    IDocumentStylePoolAccess& rIDSPA = pDoc->getIDocumentStylePoolAccess();
    SwTextFormatColl* pColl = rIDSPA.GetTextCollFromPool(SwPoolFormatId::COLL_HTML_PRE);
    pDoc->SetTextFormatColl(*pCursor, pColl);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"C"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"D"_ustr);
    pColl = rIDSPA.GetTextCollFromPool(SwPoolFormatId::COLL_STANDARD);
    pDoc->SetTextFormatColl(*pCursor, pColl);

    // When saving that to markdown:
    save(TestFilter::MD);

    // Then make sure the code block is exported:
    std::string aActual = TempFileToString();
    std::string aExpected(
        // clang-format off
        "A" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "```" SAL_NEWLINE_STRING
        "B" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "C" SAL_NEWLINE_STRING
        "```" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "D" SAL_NEWLINE_STRING
        // clang-format on
    );
    // Without the accompanying fix in place, this test would have failed with:
    // - Actual  : A\nB\nC\nD\n
    // i.e. the code block formatting was lost.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testTableColumnAdjustMdExport)
{
    // Given a document that has a table with custom adjustments:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert(u"before"_ustr);
    SwInsertTableOptions aInsertTableOptions(SwInsertTableFlags::DefaultBorder,
                                             /*nRowsToRepeat=*/0);
    pWrtShell->InsertTable(aInsertTableOptions, /*nRows=*/1, /*nCols=*/3);
    pWrtShell->Insert(u"after"_ustr);
    pWrtShell->SttPara();
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    pWrtShell->Insert(u"A1"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"B1"_ustr);
    pWrtShell->SetAttrItem(SvxAdjustItem(SvxAdjust::Center, RES_PARATR_ADJUST));
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"C1"_ustr);
    pWrtShell->SetAttrItem(SvxAdjustItem(SvxAdjust::Right, RES_PARATR_ADJUST));

    // When saving that to markdown:
    save(TestFilter::MD);

    // Then make sure the table content is not lost:
    std::string aActual = TempFileToString();
    std::string aExpected(
        // clang-format off
        "before" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "| A1 | B1 | C1 |" SAL_NEWLINE_STRING
        "| - | :-: | -: |" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "after" SAL_NEWLINE_STRING
        // clang-format on
    );
    // Without the accompanying fix in place, this test would have failed with:
    // - Actual  : | - | - | - |
    // i.e. the delimiter row's cell adjustments were lost.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testImageLinkMdImport)
{
    // Given a document with an image which has a link on it:
    // When importing that document:
    setImportFilterName(TestFilter::MD);
    createSwDoc("image-and-link.md");

    // Then make sure the link is not lost:
    SwDocShell* pDocShell = getSwDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    sw::FrameFormats<sw::SpzFrameFormat*>& rFlys = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rFlys.size());
    sw::SpzFrameFormat& rFly = *rFlys[0];
    const SwFormatURL& rURL = rFly.GetURL();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: https://www.example.com/
    // - Actual  :
    // i.e. the image's item set didn't have a URL.
    CPPUNIT_ASSERT_EQUAL(u"https://www.example.com/"_ustr, rURL.GetURL());
}

CPPUNIT_TEST_FIXTURE(Test, testImageLinkMdExport)
{
    // Given a document with an inline, linked image + link on it:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert(u"A "_ustr);
    SfxItemSet aFrameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SwFormatAnchor aAnchor(RndStdIds::FLY_AS_CHAR);
    aFrameSet.Put(aAnchor);
    Graphic aGraphic;
    OUString aGraphicURL(u"./test.png"_ustr);
    IDocumentContentOperations& rIDCO = pDoc->getIDocumentContentOperations();
    SwCursor* pCursor = pWrtShell->GetCursor();
    SwFlyFrameFormat* pFlyFormat
        = rIDCO.InsertGraphic(*pCursor, aGraphicURL, OUString(), &aGraphic, &aFrameSet,
                              /*pGrfAttrSet=*/nullptr, /*SwFrameFormat=*/nullptr);
    pFlyFormat->SetObjDescription(u"mydesc"_ustr);
    SwFormatURL aFormatURL;
    aFormatURL.SetURL(u"https://x.com"_ustr, /*bServerMap=*/false);
    pFlyFormat->SetFormatAttr(aFormatURL);
    pWrtShell->Insert(u" B"_ustr);

    // When saving that to markdown:
    save(TestFilter::MD);

    // Then make sure the image is exported and the link is not lost:
    std::string aActual = TempFileToString();
    std::string aExpected("A [![mydesc](./test.png)](https://x.com) B" SAL_NEWLINE_STRING);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: A [![mydesc](./test.png)](https://x.com) B
    // - Actual  : A ![mydesc](./test.png) B
    // i.e. the image link was lost.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testNewlineMdExport)
{
    // Given a document with a line break:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert(u"A"_ustr);
    pWrtShell->InsertLineBreak();
    pWrtShell->Insert(u"B"_ustr);

    // When saving that to markdown:
    save(TestFilter::MD);

    std::string aActual = TempFileToString();
    std::string aExpected("A  " SAL_NEWLINE_STRING "B" SAL_NEWLINE_STRING);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: A  \nB
    // - Actual  : A\nB
    // i.e. the line break was lost.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testImageDescTitleExport)
{
    // Given a document with an inline, linked image + desc/title on it:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert(u"A "_ustr);
    SfxItemSet aFrameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SwFormatAnchor aAnchor(RndStdIds::FLY_AS_CHAR);
    aFrameSet.Put(aAnchor);
    Graphic aGraphic;
    OUString aGraphicURL(u"./test.png"_ustr);
    IDocumentContentOperations& rIDCO = pDoc->getIDocumentContentOperations();
    SwCursor* pCursor = pWrtShell->GetCursor();
    SwFlyFrameFormat* pFlyFormat
        = rIDCO.InsertGraphic(*pCursor, aGraphicURL, OUString(), &aGraphic, &aFrameSet,
                              /*pGrfAttrSet=*/nullptr, /*SwFrameFormat=*/nullptr);
    pFlyFormat->SetObjDescription(u"mydesc"_ustr);
    pFlyFormat->SetObjTitle(u"mytitle"_ustr);
    pWrtShell->Insert(u" B"_ustr);

    // When saving that to markdown:
    save(TestFilter::MD);

    // Then make sure the image is exported and the desc/title is not lost:
    std::string aActual = TempFileToString();
    std::string aExpected("A ![mydesc](./test.png \"mytitle\") B" SAL_NEWLINE_STRING);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: A ![mydesc](./test.png "mytitle") B
    // - Actual  : A ![mytitle](./test.png) B
    // i.e. the title was exported as a description; the description was lost.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testMultiParaTableMdExport)
{
    // Given a document that has a table, 3 paragraphs in the A1 cell:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert(u"A"_ustr);
    SwInsertTableOptions aInsertTableOptions(SwInsertTableFlags::DefaultBorder,
                                             /*nRowsToRepeat=*/0);
    pWrtShell->InsertTable(aInsertTableOptions, /*nRows=*/3, /*nCols=*/3);
    pWrtShell->Insert(u"Z"_ustr);
    pWrtShell->SttPara();
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    pWrtShell->Insert(u"A1 first"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"A1 second"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"A1 third"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"B1"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"C1"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"A2"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"B2"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"C2"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"A3"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"B3"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"C3"_ustr);

    // When saving that to markdown:
    save(TestFilter::MD);

    // Then make sure the A1 cell still only has inlines:
    std::string aActual = TempFileToString();
    std::string aExpected(
        // clang-format off
        "A" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "| A1 first A1 second A1 third | B1 | C1 |" SAL_NEWLINE_STRING
        "| - | - | - |" SAL_NEWLINE_STRING
        "| A2 | B2 | C2 |" SAL_NEWLINE_STRING
        "| A3 | B3 | C3 |" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        "Z" SAL_NEWLINE_STRING
        // clang-format on
    );
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: A1 first A1 second A1 third
    // - Actual  : A1 first\nA1 second\n\nA1 third\n
    // i.e. multiple paragraphs were not merged into a single paragraph to form just a list of
    // inline blocks, as required by the spec.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testNestedTableMdExport)
{
    // Given a document with a nested table:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwInsertTableOptions aInsertTableOptions(SwInsertTableFlags::DefaultBorder,
                                             /*nRowsToRepeat=*/0);
    pWrtShell->InsertTable(aInsertTableOptions, /*nRows=*/2, /*nCols=*/2);
    pWrtShell->SttPara();
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    pWrtShell->Insert(u"A1 before"_ustr);
    pWrtShell->InsertTable(aInsertTableOptions, /*nRows=*/2, /*nCols=*/2);
    pWrtShell->Insert(u"A1 after"_ustr);
    pWrtShell->SttPara();
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    pWrtShell->Insert(u"A1 inner"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"B1 inner"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"A2 inner"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"B2 inner"_ustr);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"B1 outer"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"A2 outer"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"B2 outer"_ustr);

    // When saving that to markdown:
    save(TestFilter::MD);

    // Then make sure that the inner table is exported as flat paragraphs:
    std::string aActual = TempFileToString();
    std::string aExpected(
        // clang-format off
        SAL_NEWLINE_STRING
        "| A1 before A1 inner B1 inner A2 inner B2 inner A1 after | B1 outer |" SAL_NEWLINE_STRING
        "| - | - |" SAL_NEWLINE_STRING
        "| A2 outer | B2 outer |" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        // clang-format on
    );
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: | A1 before A1 inner B1 inner A2 inner B2 inner A1 after |
    // - Actual  : | A1 before \n| A1 inner | B1 inner |\n| - | - |\n| A2 inner | B2 inner |\nA1 after |
    // i.e. the outer table cell had block elements, while it is only allowed to have inlines.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testTastListItemsMdImport)
{
    // Given a document with 2 task list items:
    setImportFilterName(TestFilter::MD);

    // When importing that document from markdown:
    createSwDoc("task-list-items.md");

    // Then make sure we have two checkbox content controls, first is checked, second is not:
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    {
        SwTextContentControl* pTextContentControl = pWrtShell->CursorInsideContentControl();
        // Without the accompanying fix in place, this test would have failed, the task list item
        // was imported as a static checkbox character.
        CPPUNIT_ASSERT(pTextContentControl);
        const SwFormatContentControl& rFormatContentControl
            = pTextContentControl->GetContentControl();
        const std::shared_ptr<SwContentControl>& pContentControl
            = rFormatContentControl.GetContentControl();
        CPPUNIT_ASSERT_EQUAL(SwContentControlType::CHECKBOX, pContentControl->GetType());
        CPPUNIT_ASSERT(pContentControl->GetChecked());
    }
    pWrtShell->Down(/*bSelect=*/false, 1);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    {
        SwTextContentControl* pTextContentControl = pWrtShell->CursorInsideContentControl();
        CPPUNIT_ASSERT(pTextContentControl);
        const SwFormatContentControl& rFormatContentControl
            = pTextContentControl->GetContentControl();
        const std::shared_ptr<SwContentControl>& pContentControl
            = rFormatContentControl.GetContentControl();
        CPPUNIT_ASSERT_EQUAL(SwContentControlType::CHECKBOX, pContentControl->GetType());
        CPPUNIT_ASSERT(!pContentControl->GetChecked());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTastListItemsMdExport)
{
    // Given a document with two content control checkboxes, first is checked, second is unchecked:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::CHECKBOX);
    {
        SwTextContentControl* pTextContentControl = pWrtShell->CursorInsideContentControl();
        const SwFormatContentControl& rFormatContentControl
            = pTextContentControl->GetContentControl();
        const std::shared_ptr<SwContentControl>& pContentControl
            = rFormatContentControl.GetContentControl();
        pContentControl->SetChecked(true);
    }
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Insert(u" foo"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->InsertContentControl(SwContentControlType::CHECKBOX);
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Insert(u" bar"_ustr);

    // When saving that to markdown:
    save(TestFilter::MD);

    // Then make sure that the task list item markup is used:
    std::string aActual = TempFileToString();
    std::string aExpected("[x] foo" SAL_NEWLINE_STRING SAL_NEWLINE_STRING
                          "[ ] bar" SAL_NEWLINE_STRING);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: [x] foo\n\n[ ] bar
    // - Actual  : ☐ foo\n\n☐ bar
    // i.e. checkboxes were not written using the task list item markup.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testEmbeddedImageMdImport)
{
    // Given a document with an embedded image:
    // When importing that document:
    setImportFilterName(TestFilter::MD);
    createSwDoc("embedded-image.md");

    // Then make sure the embedded image gets imported, with the correct size:
    SwDocShell* pDocShell = getSwDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    sw::FrameFormats<sw::SpzFrameFormat*>& rFlys = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(!rFlys.empty());
    sw::SpzFrameFormat& rFly = *rFlys[0];
    const SwFormatFrameSize& rSize = rFly.GetFrameSize();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 960
    // - Actual  : 500 (MD_MIN_IMAGE_WIDTH_IN_TWIPS)
    // i.e. the image wasn't imported, had the default minimal size.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(960), rSize.GetWidth());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(960), rSize.GetHeight());
}

CPPUNIT_TEST_FIXTURE(Test, testEmbeddedImageMdExport)
{
    // Given a document with an embedded inline image:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert(u"A "_ustr);
    SfxItemSet aFrameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SwFormatAnchor aAnchor(RndStdIds::FLY_AS_CHAR);
    aFrameSet.Put(aAnchor);
    OUString aImageURL = createFileURL(u"test.png");
    SvFileStream aImageStream(aImageURL, StreamMode::READ);
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rFilter.ImportUnloadedGraphic(aImageStream);
    IDocumentContentOperations& rIDCO = pDoc->getIDocumentContentOperations();
    SwCursor* pCursor = pWrtShell->GetCursor();
    SwFlyFrameFormat* pFlyFormat
        = rIDCO.InsertGraphic(*pCursor, /*rGrfName=*/OUString(), OUString(), &aGraphic, &aFrameSet,
                              /*pGrfAttrSet=*/nullptr, /*SwFrameFormat=*/nullptr);
    pFlyFormat->SetObjDescription(u"mydesc"_ustr);
    pWrtShell->Insert(u" B"_ustr);

    // When saving that to markdown:
    save(TestFilter::MD);

    // Then make sure that the embedded image is exported:
    std::string aActual = TempFileToString();
    // Without the accompanying fix in place, this test would have failed, aActual was 'A  B\n'.
    CPPUNIT_ASSERT(aActual.starts_with("A ![mydesc](data:image/png;base64,"));
    CPPUNIT_ASSERT(aActual.ends_with(") B" SAL_NEWLINE_STRING));
}

CPPUNIT_TEST_FIXTURE(Test, testEmbeddedAnchoredImageMdExport)
{
    // Given a document with an embedded anchored image:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert(u"A "_ustr);
    SfxItemSet aFrameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SwFormatAnchor aAnchor(RndStdIds::FLY_AT_CHAR);
    aFrameSet.Put(aAnchor);
    OUString aImageURL = createFileURL(u"test.png");
    SvFileStream aImageStream(aImageURL, StreamMode::READ);
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rFilter.ImportUnloadedGraphic(aImageStream);
    IDocumentContentOperations& rIDCO = pDoc->getIDocumentContentOperations();
    SwCursor* pCursor = pWrtShell->GetCursor();
    SwFlyFrameFormat* pFlyFormat
        = rIDCO.InsertGraphic(*pCursor, /*rGrfName=*/OUString(), OUString(), &aGraphic, &aFrameSet,
                              /*pGrfAttrSet=*/nullptr, /*SwFrameFormat=*/nullptr);
    pFlyFormat->SetObjDescription(u"mydesc"_ustr);
    pWrtShell->Insert(u" B"_ustr);

    // When saving that to markdown:
    save(TestFilter::MD);

    // Then make sure that the embedded image is exported:
    std::string aActual = TempFileToString();
    // Without the accompanying fix in place, this test would have failed, aActual was 'A  B\n'.
    CPPUNIT_ASSERT(aActual.starts_with("A ![mydesc](data:image/png;base64,"));
    CPPUNIT_ASSERT(aActual.ends_with(") B" SAL_NEWLINE_STRING));
}

CPPUNIT_TEST_FIXTURE(Test, testTemplateMdImport)
{
    // Given a document with a template:

    // When importing that markdown:
    createSwDoc("template.md", {
                                   comphelper::makePropertyValue(u"FilterOptions"_ustr, uR"json({
    "TemplateURL": {
        "type": "string",
        "value": "./template.ott"
    }
})json"_ustr),
                               });

    // Then make sure the styles are taken from the template:
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwCursor* pCursor = pWrtShell->GetCursor();
    SwTextNode* pTextNode = pCursor->GetPointNode().GetTextNode();
    SwFormatColl* pStyle = pTextNode->GetFormatColl();
    auto pXFillStyleItem = pStyle->GetAttrSet().GetItem<XFillStyleItem>(XATTR_FILLSTYLE);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1 (drawing::FillStyle_SOLID)
    // - Actual  : 0 (drawing::FillStyle_NONE)
    // i.e. the heading 1 style had the default black color instead of ~blue.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, pXFillStyleItem->GetValue());
    auto pXFillColorItem = pStyle->GetAttrSet().GetItem<XFillColorItem>(XATTR_FILLCOLOR);
    CPPUNIT_ASSERT_EQUAL(Color(0x156082), pXFillColorItem->GetColorValue());
}

CPPUNIT_TEST_FIXTURE(Test, testDocxTemplateMdImport)
{
    // Given a document with a DOCX template:
    // When importing that markdown:
    // Without the accompanying fix in place, this crashed.
    createSwDoc("template.md", {
                                   comphelper::makePropertyValue(u"FilterOptions"_ustr, uR"json({
    "TemplateURL": {
        "type": "string",
        "value": "./template.docx"
    }
})json"_ustr),
                               });

    // Then make sure the styles are taken from the template:
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwCursor* pCursor = pWrtShell->GetCursor();
    SwTextNode* pTextNode = pCursor->GetPointNode().GetTextNode();
    SwFormatColl* pStyle = pTextNode->GetFormatColl();
    auto pXFillStyleItem = pStyle->GetAttrSet().GetItem<XFillStyleItem>(XATTR_FILLSTYLE);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, pXFillStyleItem->GetValue());
    auto pXFillColorItem = pStyle->GetAttrSet().GetItem<XFillColorItem>(XATTR_FILLCOLOR);
    CPPUNIT_ASSERT_EQUAL(Color(0x156082), pXFillColorItem->GetColorValue());
}

CPPUNIT_TEST_FIXTURE(Test, testOLEWithoutGraphicMdExport)
{
    // Given a document with an OLE object that has no graphic:
    createSwDoc("ole-without-graphic.odt");

    // When exporting it as markdown:
    // Then make sure this doesn't crash:
    save(TestFilter::MD);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
