/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <common/Util.hpp>

#include <Poco/JSON/Array.h>

#include <sstream>
#include <string>
#include <vector>

namespace FailurePage
{
inline const std::string style =
    "body {"
    "    font-family: sans-serif;"
    "    padding: 2em;"
    "    color: #333;"
    "    max-width: 36em;"
    "}"
    "h1 {"
    "    color: #c00;"
    "    font-size: 1.3em;"
    "}"
    "p {"
    "    line-height: 1.5;"
    "}";

inline std::string escapeHtml(const std::string& text)
{
    std::string escaped = Util::replace(text, "&", "&amp;");
    escaped = Util::replace(escaped, "<", "&lt;");
    escaped = Util::replace(escaped, ">", "&gt;");
    return Util::replace(escaped, "\"", "&quot;");
}

/// A page with a heading and paragraphs of text. Escaped text has no '<', so it never holds a
/// placeholder.
inline std::string html(const std::string& title, const std::vector<std::string>& paragraphs)
{
    static const std::string page =
        "<!doctype html>"
        "<html>"
        "<head>"
        "    <meta charset=\"utf-8\">"
        "    <title><!--title--></title>"
        "    <style><!--style--></style>"
        "</head>"
        "<body>"
        "    <h1><!--title--></h1>"
        "    <!--paragraphs-->"
        "</body>"
        "</html>";

    std::string paragraphsHtml;
    for (const std::string& paragraph : paragraphs)
        paragraphsHtml += "<p>" + escapeHtml(paragraph) + "</p>";

    std::string result = Util::replace(page, "<!--style-->", style);
    result = Util::replace(result, "<!--title-->", escapeHtml(title));
    return Util::replace(result, "<!--paragraphs-->", paragraphsHtml);
}

/// A script that replaces the page with a heading and paragraphs of text. The editor frame's
/// policy refuses an inline style, so the rules go in as a constructed sheet.
inline std::string script(const std::string& title, const std::vector<std::string>& paragraphs)
{
    Poco::JSON::Array::Ptr texts = new Poco::JSON::Array();
    for (const std::string& paragraph : paragraphs)
        texts->add(paragraph);

    Poco::JSON::Array::Ptr page = new Poco::JSON::Array();
    page->add(style);
    page->add(title);
    page->add(texts);

    std::ostringstream pageJson;
    page->stringify(pageJson);

    static const std::string showPage =
        "(([style, title, paragraphs]) => {"
        "    const sheet = new CSSStyleSheet();"
        "    sheet.replaceSync(style);"
        "    document.documentElement.innerHTML = '<head></head><body></body>';"
        "    const heading = document.createElement('h1');"
        "    heading.textContent = title;"
        "    document.body.append(heading);"
        "    for (const text of paragraphs) {"
        "        const paragraph = document.createElement('p');"
        "        paragraph.textContent = text;"
        "        document.body.append(paragraph);"
        "    }"
        "    document.adoptedStyleSheets = [sheet];"
        "})";

    return showPage + '(' + pageJson.str() + ");";
}
} // namespace FailurePage

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
