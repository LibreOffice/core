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
 * Mobile platform no-op implementations of desktop utility functions.
 * Classes: DirectoryCounter - Functions: spawnProcess(), getMemoryUsage*()
 */

#include <config.h>

#include <common/ProcUtil.hpp>
#include <common/Util.hpp>

namespace ProcUtil
{
/// No-op implementation of desktop only functions
int spawnProcess(const std::string&, const StringVector&) { return 0; }
size_t getMemoryUsagePSS(pid_t) { return 0; }
size_t getMemoryUsageRSS(pid_t) { return 0; }
size_t getCurrentThreadCount() { return 0; }
std::string getMemoryStats(FILE*) { return std::string(); }
std::pair<size_t, size_t> getPssAndDirtyFromSMaps(FILE*) { return std::make_pair(0, 0); }
std::size_t getProcessTreePss(pid_t) { return 0; }
size_t getCpuUsage(pid_t) { return 0; }
size_t getStatFromPid(pid_t, int) { return 0; }
bool isRunning(pid_t pid) { return pid > 0; }
void setProcessAndThreadPriorities(pid_t, int) {}
} // namespace ProcUtil

namespace Util
{
std::string getHumanizedBytes(unsigned long) { return std::string(); }
size_t getTotalSystemMemoryKb() { return 0; }
std::size_t getFromFile(const char*) { return 0; }
std::size_t getCGroupMemLimit() { return 0; }
std::size_t getCGroupMemSoftLimit() { return 0; }
std::size_t getCGroupOomKillCount() { return 0; }
std::size_t getKeyedFromFile(const std::string&, const std::string&) { return 0; }

std::string getLinuxVersion() { return "unknown"; }

void alertAllUsers(const std::string&) {}
void alertAllUsers(const std::string&, const std::string&) {}
} // namespace Util

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
