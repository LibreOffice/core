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
        "## Heading 2" SAL_NEWLINE_STRING
        "### Heading 3" SAL_NEWLINE_STRING
        "#### Heading 4" SAL_NEWLINE_STRING
        "##### Heading 5" SAL_NEWLINE_STRING
        "###### Heading 6" SAL_NEWLINE_STRING
        "**Bold** text" SAL_NEWLINE_STRING
        "Text in *italics*" SAL_NEWLINE_STRING
        "This is a [hyperlink](http://www.libreoffice.org/)" SAL_NEWLINE_STRING
        "\\# Leading hash" SAL_NEWLINE_STRING
        "Some \\{braces\\}, \\[square brackets\\], \\*asterisks\\*, \\`backticks\\`, \\\\backslashes\\\\, \\_underscores\\_, \\<angle brackets\\>" SAL_NEWLINE_STRING
        SAL_NEWLINE_STRING
        // clang-format on
    );

    CPPUNIT_ASSERT_EQUAL(expected, md_content);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
