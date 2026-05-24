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

/*
 * Content Security Policy header generation for HTTP responses.
 * Classes: ContentSecurityPolicy
 */

#pragma once

#include <common/ContainerUtil.hpp>
#include <common/Log.hpp>
#include <common/StringVector.hpp>
#include <common/Util.hpp>
#include <sstream>
#include <string>

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
        // Replace newlines with spaces since StringVector::tokenize
        // with a char delimiter stops at the first newline.
        std::string cspLine = csp;
        std::replace(cspLine.begin(), cspLine.end(), '\n', ' ');
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

    /// Append the given URL to a directive.
    /// @value must be space-delimited and cannot have semicolon.
    void appendDirectiveUrl(std::string directive, const std::string& url)
    {
        appendDirective(std::move(directive), Util::trimURI(url));
    }

    /// Append the given value to a directive.
    /// @value must be space-delimited and cannot have semicolon.
    void appendDirective(std::string directive, std::string value)
    {
        if (value.find_first_of(';') != std::string::npos)
        {
            LOG_WRN("Unexpected semicolon in CSP source [" << value << "] for policy directive ["
                    << directive << "] - ignoring it.");
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
