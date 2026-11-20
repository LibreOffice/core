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

#pragma once

#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/StringVector.hpp>
#include <common/Uri.hpp>

#include <Poco/Exception.h>

#include <array>
#include <cstddef>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/// The options of the load command, in alphabetical order.
enum class LoadOption
{
    AccessibilityState,
    AllowChangeComments,
    AllowManageRedlines,
    AnonymizedAuthor,
    AnonymizedAuthorId,
    AnonymizedJail,
    Author,
    AuthorExtraInfo,
    AuthorId,
    AuthorPrivateInfo,
    Batch,
    ClientVisibleArea,
    DarkBackground,
    DarkTheme,
    DeviceFormFactor,
    EnableMacrosExecution,
    FocusRingColor,
    FormattingMarks,
    InFilterOptions,
    Jail,
    Lang,
    MacroSecurityLevel,
    OriginalDocumentUrl,
    Part,
    Password,
    ReadOnly,
    RemoteLink,
    ServerPrivateInfo,
    SignatureConfig,
    SpellOnline,
    Template,
    ThumbnailSizes,
    Timezone,
    Url,
    VectorRendering,
    VerifyHost,
    WatermarkOpacity,
    WatermarkText,
    Last = WatermarkText
};

struct LoadOptionInfo
{
    LoadOption option;
    /// The name of the option in the text of the load command.
    std::string_view name;
    /// The check for a value that comes from a browser. Null when only the server sets the option.
    bool (*isValidBrowserValue)(std::string_view);
};

/// The load command: the word load and then a name=value token for each option that is set, in the
/// order of LoadOption. Every value is percent-encoded when the text is written and decoded when
/// it is parsed, so a value holds any text and stays one token.
class LoadCommand
{
    /// The letters are the ASCII ones.
    static bool hasOnlyLettersDigitsAnd(std::string_view text, std::string_view extra)
    {
        for (const char c : text)
        {
            const bool letterOrDigit =
                (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
            if (!letterOrDigit && extra.find(c) == std::string_view::npos)
                return false;
        }
        return true;
    }

    static bool isAnyText(std::string_view) { return true; }

    static bool isBool(std::string_view text)
    {
        return text.empty() || text == "true" || text == "false";
    }

    /// A language tag such as en-US or ca-valencia, or a locale name such as en_US.UTF-8.
    static bool isLanguage(std::string_view text) { return hasOnlyLettersDigitsAnd(text, "-_.@"); }

    /// A name from the time zone database, such as America/Argentina/Buenos_Aires or Etc/GMT+1.
    static bool isTimezone(std::string_view text) { return hasOnlyLettersDigitsAnd(text, "/_+-"); }

    static bool isDeviceFormFactor(std::string_view text)
    {
        return text.empty() || text == "desktop" || text == "tablet" || text == "mobile";
    }

    /// Either true or false, or a choice for each document type, such as
    /// text:true,spreadsheet:false.
    static bool isSpellOnline(std::string_view text) { return hasOnlyLettersDigitsAnd(text, ":,"); }

    /// A colour as # and six hexadecimal digits.
    static bool isColor(std::string_view text)
    {
        if (text.empty())
            return true;
        if (text.size() != 7 || text[0] != '#')
            return false;
        return text.find_first_not_of("0123456789abcdefABCDEF", 1) == std::string_view::npos;
    }

    /// Digits, with an optional minus sign before them and an optional fraction after them.
    static bool isNumber(std::string_view text)
    {
        if (text.starts_with('-'))
            text.remove_prefix(1);
        const std::size_t point = text.find('.');
        const std::string_view whole = text.substr(0, point);
        const std::string_view fraction =
            point == std::string_view::npos ? "0" : text.substr(point + 1);
        return !whole.empty() && !fraction.empty() &&
               whole.find_first_not_of("0123456789") == std::string_view::npos &&
               fraction.find_first_not_of("0123456789") == std::string_view::npos;
    }

    /// Four numbers separated by semicolons, such as 0;0;10980;2250.
    static bool isArea(std::string_view text)
    {
        if (text.empty())
            return true;
        int numbers = 0;
        while (true)
        {
            const std::size_t end = text.find(';');
            if (!isNumber(text.substr(0, end)))
                return false;
            ++numbers;
            if (end == std::string_view::npos)
                return numbers == 4;
            text.remove_prefix(end + 1);
        }
    }

public:
    using Options = std::vector<std::pair<LoadOption, std::string>>;

    static constexpr std::size_t OptionCount = static_cast<std::size_t>(LoadOption::Last) + 1;

    static constexpr std::array<LoadOptionInfo, OptionCount> Table{ {
        { LoadOption::AccessibilityState, "accessibilityState", isBool },
        { LoadOption::AllowChangeComments, "isAllowChangeComments", nullptr },
        { LoadOption::AllowManageRedlines, "isAllowManageRedlines", nullptr },
        { LoadOption::AnonymizedAuthor, "xauthor", nullptr },
        { LoadOption::AnonymizedAuthorId, "xauthorid", nullptr },
        { LoadOption::AnonymizedJail, "xjail", nullptr },
        { LoadOption::Author, "author", nullptr },
        { LoadOption::AuthorExtraInfo, "authorextrainfo", nullptr },
        { LoadOption::AuthorId, "authorid", nullptr },
        { LoadOption::AuthorPrivateInfo, "authorprivateinfo", nullptr },
        { LoadOption::Batch, "batch", isBool },
        { LoadOption::ClientVisibleArea, "clientvisiblearea", isArea },
        { LoadOption::DarkBackground, "darkBackground", isBool },
        { LoadOption::DarkTheme, "darkTheme", isBool },
        { LoadOption::DeviceFormFactor, "deviceFormFactor", isDeviceFormFactor },
        { LoadOption::EnableMacrosExecution, "enableMacrosExecution", nullptr },
        { LoadOption::FocusRingColor, "focusRingColor", isColor },
        { LoadOption::FormattingMarks, "formattingMarks", isBool },
        { LoadOption::InFilterOptions, "infilterOptions", nullptr },
        { LoadOption::Jail, "jail", nullptr },
        { LoadOption::Lang, "lang", isLanguage },
        { LoadOption::MacroSecurityLevel, "macroSecurityLevel", nullptr },
        { LoadOption::OriginalDocumentUrl, "originaldocumenturl", nullptr },
        { LoadOption::Part, "part", COOLProtocol::isValidPartId },
        { LoadOption::Password, "password", isAnyText },
        { LoadOption::ReadOnly, "readonly", nullptr },
        { LoadOption::RemoteLink, "remotelink", nullptr },
        { LoadOption::ServerPrivateInfo, "serverprivateinfo", nullptr },
        { LoadOption::SignatureConfig, "signatureconfig", nullptr },
        { LoadOption::SpellOnline, "spellOnline", isSpellOnline },
        { LoadOption::Template, "template", nullptr },
        { LoadOption::ThumbnailSizes, "thumbnailSizes", nullptr },
        { LoadOption::Timezone, "timezone", isTimezone },
        { LoadOption::Url, "url", isAnyText },
        { LoadOption::VectorRendering, "vectorRendering", isBool },
        { LoadOption::VerifyHost, "verifyHost", nullptr },
        { LoadOption::WatermarkOpacity, "watermarkOpacity", nullptr },
        { LoadOption::WatermarkText, "watermarkText", nullptr },
    } };

    static constexpr bool isTableInEnumOrder()
    {
        for (std::size_t i = 0; i < Table.size(); ++i)
        {
            if (static_cast<std::size_t>(Table[i].option) != i)
                return false;
        }
        return true;
    }

    static constexpr const LoadOptionInfo& getOptionInfo(LoadOption option)
    {
        return Table[static_cast<std::size_t>(option)];
    }

    /// True when a browser may set the option and the text passes the check in its row.
    static bool isValidBrowserValue(LoadOption option, std::string_view text)
    {
        const LoadOptionInfo& info = getOptionInfo(option);
        return info.isValidBrowserValue && info.isValidBrowserValue(text);
    }

    static constexpr const LoadOptionInfo* findOption(std::string_view name)
    {
        for (const LoadOptionInfo& info : Table)
        {
            if (info.name == name)
                return &info;
        }
        return nullptr;
    }

    void set(LoadOption option, std::string value)
    {
        _values[static_cast<std::size_t>(option)] = std::move(value);
    }

    void set(LoadOption option, const char* value) { set(option, std::string(value)); }

    void set(LoadOption option, bool value)
    {
        set(option, std::string(value ? "true" : "false"));
    }

    void set(LoadOption option, int value) { set(option, std::to_string(value)); }

    void set(LoadOption option, double value)
    {
        std::ostringstream oss;
        oss << value;
        set(option, oss.str());
    }

    std::string serialize() const
    {
        std::string command = "load";
        for (std::size_t i = 0; i < _values.size(); ++i)
        {
            if (!_values[i])
                continue;
            command += ' ';
            command += Table[i].name;
            command += '=';
            command += Uri::encode(*_values[i], std::string());
        }
        return command;
    }

    /// The options of a tokenized load command, each with its decoded value. Empty when the first
    /// token is not the word load. Leaves out a token that names no option, a second token for an
    /// option, and a value that does not decode.
    static Options parse(const StringVector& tokens)
    {
        Options options;
        if (!tokens.equals(0, "load"))
        {
            LOG_ERR("Expected a load command");
            return options;
        }

        std::array<bool, OptionCount> seen{};
        for (std::size_t i = 1; i < tokens.size(); ++i)
        {
            std::string name;
            std::string value;
            if (!COOLProtocol::parseNameValuePair(tokens[i], name, value))
            {
                LOG_WRN("Unexpected load token [" << tokens[i] << "]. Skipping.");
                continue;
            }

            const LoadOptionInfo* info = findOption(name);
            if (!info)
            {
                LOG_WRN("Ignoring the unknown load option [" << name << ']');
                continue;
            }

            bool& alreadySeen = seen[static_cast<std::size_t>(info->option)];
            if (alreadySeen)
            {
                LOG_WRN("Ignoring a repeat of the load option [" << name << ']');
                continue;
            }
            alreadySeen = true;

            try
            {
                options.emplace_back(info->option, Uri::decode(value));
            }
            catch (const Poco::Exception& exception)
            {
                LOG_WRN("Ignoring the load option [" << name << "], its value does not decode: "
                                                     << exception.displayText());
            }
        }
        return options;
    }

private:
    std::array<std::optional<std::string>, OptionCount> _values;
};

static_assert(LoadCommand::isTableInEnumOrder(),
              "The rows of LoadCommand::Table follow the order of LoadOption");

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
