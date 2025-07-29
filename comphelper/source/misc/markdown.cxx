/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/markdown.hxx>

#include <unicode/regex.h>

constexpr sal_Int32 MAX_SEARCH_CHARS = 2000;

namespace comphelper
{
bool IsMarkdownData(OUString& rData)
{
    icu::UnicodeString ustr(rData.getStr(), std::min(rData.getLength(), MAX_SEARCH_CHARS));

    UErrorCode status = U_ZERO_ERROR;

    static std::unique_ptr<icu::RegexPattern> patternHeadings(
        icu::RegexPattern::compile(uR"(^\s*#{1,6}\s+.*)", UREGEX_MULTILINE, status));

    static std::unique_ptr<icu::RegexPattern> patternOLists(
        icu::RegexPattern::compile(uR"(^\s*\d+\.\s+.+)", UREGEX_MULTILINE, status));

    static std::unique_ptr<icu::RegexPattern> patternULists(
        icu::RegexPattern::compile(uR"(^\s*[-*+]\s+.*)", UREGEX_MULTILINE, status));

    static std::unique_ptr<icu::RegexPattern> patternLinks(
        icu::RegexPattern::compile(uR"(\[.*?\]\(.*?\))", 0, status));

    static std::unique_ptr<icu::RegexPattern> patternCode(
        icu::RegexPattern::compile(uR"(```[\s\S]*?```)", 0, status));

    static std::unique_ptr<icu::RegexPattern> patternItalic(
        icu::RegexPattern::compile(uR"(([*_])(?!\s)(.+?)(?<!\s)\1)", 0, status));

    static std::unique_ptr<icu::RegexPattern> patternBold(
        icu::RegexPattern::compile(uR"(([*_])\1(?!\s)(.+?)(?<!\s)\1\1)", 0, status));

    static std::unique_ptr<icu::RegexPattern> patternStrikethrough(
        icu::RegexPattern::compile(uR"(~~(?!\s)(.+?)(?<!\s)~~)", 0, status));

    static std::unique_ptr<icu::RegexPattern> patternHorizontalRule(
        icu::RegexPattern::compile(uR"(^(?:-{3,}|\*{3,}|_{3,})$)", UREGEX_MULTILINE, status));

    static std::unique_ptr<icu::RegexPattern> patternBlockQuotes(
        icu::RegexPattern::compile(uR"(^\s*>+.*$)", UREGEX_MULTILINE, status));

    static std::unique_ptr<icu::RegexPattern> patternTables(icu::RegexPattern::compile(
        uR"(^\s*\|?(?:[^|\n]+?\|)+[^|\n]*\|?\s*$)", UREGEX_MULTILINE, status));

    if (U_FAILURE(status))
        return false;

    return patternHeadings->matcher(ustr, status)->find()
           || patternOLists->matcher(ustr, status)->find()
           || patternULists->matcher(ustr, status)->find()
           || patternLinks->matcher(ustr, status)->find()
           || patternCode->matcher(ustr, status)->find()
           || patternItalic->matcher(ustr, status)->find()
           || patternBold->matcher(ustr, status)->find()
           || patternStrikethrough->matcher(ustr, status)->find()
           || patternHorizontalRule->matcher(ustr, status)->find()
           || patternBlockQuotes->matcher(ustr, status)->find()
           || patternTables->matcher(ustr, status)->find();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
