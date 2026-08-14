/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * UI command logging for debugging and replay.
 * Classes: LogUiCmd - Command logging and file management
 */

#include <config.h>

#include <kit/LogUI.hpp>

#include <common/FileUtil.hpp>
#include <common/Log.hpp>

void LogUiCmd::logUiCmdLine(int userId, const std::string& line)
{
    _fileStreamUICommands.write(line.c_str(), line.length());
    _fileStreamUICommands.write("\n", 1);
    if (userId>=0)
        _usersLogged.insert(userId);
}

void LogUiCmd::saveLogFile()
{
    std::string timeLog = "log-start-time: " + _kitStartTimeStr + " kit=" + _docId + " user-count:" + std::to_string(_usersLogged.size());
    Log::logUI(Log::WRN, timeLog);
    _fileStreamUICommands.seekg(0, std::ios::beg);
    std::string line;
    while (std::getline(_fileStreamUICommands, line))
    {
        if (line.size()>0)
            Log::logUI(Log::WRN, line);
    }
    _fileStreamUICommands.close();
    timeLog = "log-end-time: ";
    timeLog.append(Util::getTimeNow("%Y-%m-%d %T"));
    timeLog += " kit=" + _docId;
    Log::logUI(Log::WRN, timeLog);
}

void LogUiCmd::createTmpFile(const std::string& docId)
{
    const std::string tempFile = FileUtil::getSysTempDirectoryPath() + "/kit-ui-cmd.log";
    _fileStreamUICommands.open(tempFile, std::fstream::in | std::fstream::out | std::fstream::trunc);
    if (!_fileStreamUICommands)
        LOG_WRN("Failed to open the UI command log [" << tempFile << "], so it stays empty");
    _kitStartTimeSec = std::chrono::steady_clock::now();
    _kitStartTimeStr = Util::getTimeNow("%Y-%m-%d %T");
    _docId = docId;
}

std::chrono::steady_clock::time_point LogUiCmd::getKitStartTimeSec()
{
    return _kitStartTimeSec;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
