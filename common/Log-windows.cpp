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

#include <config.h>

#include <ctime>
#include <iomanip>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Log.hpp"
#include "Util.hpp"

namespace Log
{
Level level;
static std::string levelName;

void initialize(const std::string& name, const std::string& logLevel, const bool withColor,
                const bool logToFile, const std::map<std::string, std::string>& config,
                const bool logToFileUICmd, const std::map<std::string, std::string>& configUICmd)
{
    isDebuggerPresent = IsDebuggerPresent();

    setLevel(logLevel);

    const std::time_t t = std::time(nullptr);
    struct tm tm;
    LOG_INF("Initializing " << name << ". UTC time: "
                            << std::put_time(Util::time_t_to_gmtime(t, tm), "%a %F %T")
                            << ". Log level is [" << getLevelName() << ']');
}

void shutdown() {}

void flush() {}

void setThreadLocalLogLevel(const std::string& logLevel)
{
    // Don't bother with thread-local log levels here
    setLevel(logLevel);
}

bool isEnabled(Level l, Area a)
{
    if (!isDebuggerPresent)
        return false;

    if (level < l)
        return false;

    return true;
}

void log(Level l, const std::string& text)
{
    if (!isDebuggerPresent)
        return;

    auto wtext = Util::string_to_wide_string(text);
    OutputDebugStringW((wtext + L"\n").c_str());
}

bool isLogUIEnabled() { return false; }

void logUI(Level, const std::string&) {}

bool isLogUIMerged() { return false; }

bool isLogUITimeEnd() { return false; }

void setLevel(const std::string& logLevel)
{
    levelName = logLevel;
    if (logLevel == "fatal")
        level = FTL;
    else if (logLevel == "critical")
        level = CTL;
    else if (logLevel == "error")
        level = ERR;
    else if (logLevel == "warning")
        level = WRN;
    else if (logLevel == "notice")
        level = NTC;
    else if (logLevel == "information")
        level = INF;
    else if (logLevel == "debug")
        level = DBG;
    else if (logLevel == "trace")
        level = TRC;
    else
    {
        level = FTL;
        levelName = "fatal";
    }
}

void setDisabledAreas(const std::string &areaStr) { }

Level getLevel() { return level; }

const std::string& getLevelName() { return levelName; }
} // namespace Log

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
