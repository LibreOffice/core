/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Content Security Policy header generation for HTTP responses.
 * Classes: ContentSecurityPolicy
 */

#pragma once

#include <common/ContainerUtil.hpp>
#include <common/Log.hpp>
#include <common/StringVector.hpp>
#include <common/Util.hpp>
#include <algorithm>
#include <sstream>
#include <string>
#include <string_view>

/// Manages the HTTP Content-Security-Policy Header.
/// See https://www.w3.org/TR/CSP2/
class ContentSecurityPolicy
{
public:
    ContentSecurityPolicy() = default;

    ContentSecurityPolicy(const ContentSecurityPolicy& other)
        : _directives(other._directives)
    {
    }

    /// Construct a CSP from a CSP string.
    ContentSecurityPolicy(const std::string& csp)
    {
        merge(csp);
    }

    /// Given a CSP string, merge it with the existing values.
    void merge(const std::string& csp)
    {
        LOG_TRC("Merging CSP directives [" << csp << ']');
        // Replace newlines and carriage returns with spaces since StringVector::tokenize
        // with a char delimiter stops at the first newline.
        std::string cspLine = csp;
        std::replace_if(
            cspLine.begin(), cspLine.end(),
            [](const char ch) -> bool { return ch == '\n' || ch == '\r'; }, ' ');
        StringVector tokens = StringVector::tokenize(std::move(cspLine), ';');
        for (std::size_t i = 0; i < tokens.size(); ++i)
        {
            const std::string token = Util::trimmed(tokens[i]);
            if (!token.empty())
            {
                LOG_TRC("Merging CSP directive [" << token << ']');
                const auto parts = Util::split(token);
                appendDirective(std::string(parts.first), std::string(parts.second));
            }
        }
    }

    /// Given a CSP object, merge it with the existing values.
    void merge(const ContentSecurityPolicy& csp)
    {
        LOG_TRC("Merging CSP object");
        for (const auto& directive : csp._directives) {
            appendDirective(directive.first, directive.second);
        }
    }

    /// True when the string holds only bytes one source expression can carry. Whitespace ends
    /// the source, a semicolon ends the directive, a comma ends the policy, and a control byte
    /// ends the header field.
    [[nodiscard]] static bool hasOnlyValidSourceBytes(const std::string_view source)
    {
        return std::none_of(source.begin(), source.end(),
                            [](const char ch) -> bool
                            {
                                const unsigned char byte = static_cast<unsigned char>(ch);
                                return byte <= 0x20 || byte == 0x7f || byte == ';' ||
                                       byte == ',';
                            });
    }

    /// True when every source in the space-delimited list is one the policy can carry.
    [[nodiscard]] static bool hasOnlyValidSources(const std::string_view value)
    {
        for (std::string_view rest = value; !rest.empty();)
        {
            const std::size_t space = rest.find(' ');
            const std::string_view source = rest.substr(0, space);
            if (!source.empty() && !hasOnlyValidSourceBytes(source))
                return false;

            if (space == std::string_view::npos)
                break;

            rest = rest.substr(space + 1);
        }

        return true;
    }

    /// Append the given URL to a directive.
    /// @value must be space-delimited and cannot have semicolon.
    void appendDirectiveUrl(std::string directive, const std::string& url)
    {
        std::string source = Util::trimURI(url);
        if (!hasOnlyValidSourceBytes(source))
        {
            LOG_WRN("Bad byte in CSP source URL for policy directive [" << directive
                    << "] - ignoring it.");
            return;
        }

        appendDirective(std::move(directive), std::move(source));
    }

    /// Append the given value to a directive.
    /// @value must be space-delimited and cannot have semicolon.
    void appendDirective(std::string directive, std::string value)
    {
        if (!hasOnlyValidSources(value))
        {
            LOG_WRN("Bad byte in a CSP source for policy directive [" << directive
                    << "] - ignoring it.");
            return;
        }

        Util::trim(directive);
        Util::trim(value);
        if (!directive.empty() && !value.empty())
        {
            LOG_TRC("Appending CSP directive [" << directive << "] = [" << value << ']');
            _directives[directive].append(' ' + value);
        }
    }

    /// Return an individual policy.
    std::string getDirective(const std::string& directive) const
    {
        auto csp = _directives.find(directive);
        if (csp == _directives.end())
        {
            return "";
        }
        return csp->second;
    }

    /// Returns the value of the CSP header.
    std::string generate() const
    {
        std::ostringstream oss;
        for (const auto& pair : _directives)
        {
            oss << pair.first << ' ' << pair.second << "; ";
        }

        return oss.str();
    }

private:
    /// The policy directives.
    Util::UnorderedStringMap<std::string> _directives;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
