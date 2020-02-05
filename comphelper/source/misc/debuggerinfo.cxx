/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/debuggerinfo.hxx>

#include <cassert>
#include <cstring>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined UNX
#include <unistd.h>
#include <fcntl.h>
#endif

namespace comphelper
{
#if defined DBG_UTIL && !defined NDEBUG
bool isDebuggerAttached()
{
#if defined(_WIN32)
    return IsDebuggerPresent();
#elif defined LINUX
    char buf[4096];
    int fd = open("/proc/self/status", O_RDONLY);
    if (fd < 0)
        return false;
    int size = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (size < 0)
        return false;
    assert(size < int(sizeof(buf)) - 1);
    buf[sizeof(buf) - 1] = '\0';
    // "TracerPid: <pid>" for pid != 0 means something is attached
    const char* pos = strstr(buf, "TracerPid:");
    if (pos == nullptr)
        return false;
    pos += strlen("TracerPid:");
    while (*pos != '\n' && isspace(*pos))
        ++pos;
    return *pos != '\n' && *pos != '0';
#else
    return false; // feel free to add your platform
#endif
}
#endif

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
