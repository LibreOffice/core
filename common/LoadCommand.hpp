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
};

/// The load command: the word load and then a name=value token for each option that is set, in the
/// order of LoadOption. Every value is percent-encoded when the text is written and decoded when
/// it is parsed, so a value holds any text and stays one token.
class LoadCommand
{
public:
    using Options = std::vector<std::pair<LoadOption, std::string>>;

    static constexpr std::size_t OptionCount = static_cast<std::size_t>(LoadOption::Last) + 1;

    static constexpr std::array<LoadOptionInfo, OptionCount> Table{ {
        { LoadOption::AccessibilityState, "accessibilityState" },
        { LoadOption::AllowChangeComments, "isAllowChangeComments" },
        { LoadOption::AllowManageRedlines, "isAllowManageRedlines" },
        { LoadOption::AnonymizedAuthor, "xauthor" },
        { LoadOption::AnonymizedAuthorId, "xauthorid" },
        { LoadOption::AnonymizedJail, "xjail" },
        { LoadOption::Author, "author" },
        { LoadOption::AuthorExtraInfo, "authorextrainfo" },
        { LoadOption::AuthorId, "authorid" },
        { LoadOption::AuthorPrivateInfo, "authorprivateinfo" },
        { LoadOption::Batch, "batch" },
        { LoadOption::ClientVisibleArea, "clientvisiblearea" },
        { LoadOption::DarkBackground, "darkBackground" },
        { LoadOption::DarkTheme, "darkTheme" },
        { LoadOption::DeviceFormFactor, "deviceFormFactor" },
        { LoadOption::EnableMacrosExecution, "enableMacrosExecution" },
        { LoadOption::FocusRingColor, "focusRingColor" },
        { LoadOption::FormattingMarks, "formattingMarks" },
        { LoadOption::InFilterOptions, "infilterOptions" },
        { LoadOption::Jail, "jail" },
        { LoadOption::Lang, "lang" },
        { LoadOption::MacroSecurityLevel, "macroSecurityLevel" },
        { LoadOption::OriginalDocumentUrl, "originaldocumenturl" },
        { LoadOption::Part, "part" },
        { LoadOption::Password, "password" },
        { LoadOption::ReadOnly, "readonly" },
        { LoadOption::RemoteLink, "remotelink" },
        { LoadOption::ServerPrivateInfo, "serverprivateinfo" },
        { LoadOption::SignatureConfig, "signatureconfig" },
        { LoadOption::SpellOnline, "spellOnline" },
        { LoadOption::Template, "template" },
        { LoadOption::ThumbnailSizes, "thumbnailSizes" },
        { LoadOption::Timezone, "timezone" },
        { LoadOption::Url, "url" },
        { LoadOption::VectorRendering, "vectorRendering" },
        { LoadOption::VerifyHost, "verifyHost" },
        { LoadOption::WatermarkOpacity, "watermarkOpacity" },
        { LoadOption::WatermarkText, "watermarkText" },
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
