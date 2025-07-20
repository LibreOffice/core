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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
