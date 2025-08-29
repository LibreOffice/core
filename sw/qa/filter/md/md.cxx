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

#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <poolfmt.hxx>
#include <charatr.hxx>

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
        : SwModelTestBase(u"/sw/qa/filter/md/data/"_ustr, u"Markdown"_ustr)
    {
    }
};
}

CPPUNIT_TEST_FIXTURE(Test, testExportingBasicElements)
{
    createSwDoc("basic-elements.fodt");

    save(mpFilter);
    SvFileStream fileStream(maTempFile.GetURL(), StreamMode::READ);
    std::vector<char> buffer(fileStream.remainingSize());
    fileStream.ReadBytes(buffer.data(), buffer.size());
    std::string_view md_content(buffer.data(), buffer.size());
    std::string_view expected(
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

    CPPUNIT_ASSERT_EQUAL(expected, md_content);
}

CPPUNIT_TEST_FIXTURE(Test, testHeading)
{
    setImportFilterName("Markdown");
    createSwDoc("heading.md");

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1),
                         getProperty<sal_Int16>(getParagraph(1), u"OutlineLevel"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testList)
{
    setImportFilterName("Markdown");
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
    save(mpFilter);
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
    setImportFilterName("Markdown");
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
    SwTextFormatColl* pColl = rIDSPA.GetTextCollFromPool(RES_POOLCOLL_HTML_PRE);
    SfxItemSetFixed<RES_CHRATR_BEGIN, RES_CHRATR_END> aSet(rView.GetPool());
    aSet.Put(pColl->GetFont());
    pWrtShell->SetAttrSet(aSet);

    // When saving that to markdown:
    save(mpFilter);

    // Then make sure the format of B is exported:
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    std::vector<char> aBuffer(aStream.remainingSize());
    aStream.ReadBytes(aBuffer.data(), aBuffer.size());
    std::string_view aActual(aBuffer.data(), aBuffer.size());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: A `B` C
    // - Actual  : A B C
    // i.e. the code formatting was lost.
    std::string_view aExpected("A `B` C" SAL_NEWLINE_STRING);
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
    save(mpFilter);

    // Then make sure list type and level is exported:
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    std::vector<char> aBuffer(aStream.remainingSize());
    aStream.ReadBytes(aBuffer.data(), aBuffer.size());
    std::string_view aActual(aBuffer.data(), aBuffer.size());
    std::string_view aExpected(
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
