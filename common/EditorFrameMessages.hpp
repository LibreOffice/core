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

#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/StringVector.hpp>
#include <common/Util.hpp>

#include <algorithm>
#include <iterator>
#include <string>
#include <string_view>

/// The messages CODA's editor page in an integrator's editor frame may send to native code. The
/// frame edits one remote document, so nothing names a local file or opens another document.
namespace EditorFrameMessages
{
/// Allowed app commands from the editor page.
inline constexpr std::string_view ALLOWED_APP_COMMANDS[] = {
    "BYE",                 "CLOSE_CANCELLED", "CLOSE_WINDOW", "COMMANDRESULT",
    "COMMANDSTATECHANGED", "FOCUSIFHWKBD",    "FONTPICKER",   "HULLO",
    "HYPERLINK",           "LICENSE",         "PRINT",        "REMOTEBOOTSTRAPFAILED",
    "SAVECOMPLETED",       "SAVEFAILED",      "SAVESTARTED",  "SETDARKMODE",
    "TEXTCLIPBOARD",       "WELCOME",
};

/// Commands that name a local file or open another document.
inline constexpr std::string_view REFUSED_COMMANDS[] = {
    "exportfile", "extensionsavefile", "insertfilelist",     "loaddocument",       "newdoc",
    "opendoc",    "openremote",        "removecloudaccount",
};

/// Bridge calls the editor page makes, by name.
/// TODO: Qt's CloudAccountPage::callBridge handles these by name; let it use this list too.
inline constexpr std::string_view ALLOWED_BRIDGE_CALLS[] = {
    "checkFileInfo", "getAllPrefs", "getBundledFile",     "readLocalDocBytes",
    "saveCompleted", "setPref",     "writeRemoteDocFile",
};

/// URL schemes that name a local or network file, or a file of the engine itself.
inline constexpr std::string_view FILE_SCHEMES[] = {
    "file:", "ftp:", "sftp:", "smb:", "vnd.sun.star.",
};

/// Whether the editor frame may send message, a command followed by its arguments.
inline bool isAllowed(const std::string& message)
{
    const StringVector tokens = StringVector::tokenize(message);
    if (tokens.empty())
    {
        LOG_WRN("editor frame: refused: an empty message");
        return false;
    }

    const std::string command = tokens[0];
    const bool isRefused = std::find(std::begin(REFUSED_COMMANDS), std::end(REFUSED_COMMANDS),
                                     command) != std::end(REFUSED_COMMANDS);
    if (isRefused)
    {
        LOG_WRN("editor frame: refused: " << command
                                          << " names a local file or opens another document");
        return false;
    }

    const auto isCapitalOrUnderscore = [](char c) { return (c >= 'A' && c <= 'Z') || c == '_'; };
    const bool isAppCommand = std::all_of(command.begin(), command.end(), isCapitalOrUnderscore);
    const bool isListed = std::find(std::begin(ALLOWED_APP_COMMANDS),
                                    std::end(ALLOWED_APP_COMMANDS),
                                    command) != std::end(ALLOWED_APP_COMMANDS);
    if (isAppCommand && !isListed)
    {
        LOG_WRN("editor frame: refused: " << command << " is not allowed");
        return false;
    }

    if (command == "HYPERLINK")
    {
        // Only web and mail addresses are allowed.
        const std::string address = tokens.substrFromToken(1);
        const std::string scheme = Util::toLower(address.substr(0, address.find(':')));
        const bool isWebAddress = scheme == "https" || scheme == "http" || scheme == "mailto";
        if (!isWebAddress)
        {
            LOG_WRN("editor frame: refused: HYPERLINK to " << address << " is not a web address");
            return false;
        }

        return true;
    }

    if (command == "saveas")
    {
        std::string url;
        COOLProtocol::getTokenString(tokens, "url", url);
        if (!Util::toLower(url).starts_with("wopi:"))
        {
            LOG_WRN("editor frame: refused: saveas to " << url << " is not to the WOPI host");
            return false;
        }

        return true;
    }

    if (command == "uno")
    {
        std::string unoCommand;
        if (tokens.size() > 1)
            unoCommand = tokens[1];

        if (!unoCommand.starts_with(".uno:"))
        {
            LOG_WRN("editor frame: refused: uno " << unoCommand << " is a script or a macro");
            return false;
        }

        // Without a file argument, a command that opens or saves a file shows a file dialog.
        const std::string lowerMessage = Util::toLower(message);

        for (std::string_view scheme : FILE_SCHEMES)
        {
            if (lowerMessage.find(scheme) != std::string::npos)
            {
                LOG_WRN("editor frame: refused: uno " << unoCommand << " names a file");
                return false;
            }
        }

        return true;
    }

    return true;
}

inline bool isAllowedCall(const std::string& name)
{
    for (std::string_view allowed : ALLOWED_BRIDGE_CALLS)
        if (name == allowed)
            return true;

    LOG_WRN("editor frame: refused: the call " << name << " is not allowed");
    return false;
}
} // namespace EditorFrameMessages

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
