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
#include <ctype.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined MACOSX
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#elif defined UNX
#include <unistd.h>
#include <fcntl.h>
#endif

namespace comphelper
{
#if defined DBG_UTIL
bool isDebuggerAttached()
{
#if defined(_WIN32)
    return IsDebuggerPresent();
#elif defined MACOSX
    // https://developer.apple.com/library/archive/qa/qa1361/_index.html
    int                 junk;
    int                 mib[4];
    struct kinfo_proc   info;
    size_t              size;

    // Initialize the flags so that, if sysctl fails for some bizarre
    // reason, we get a predictable result.

    info.kp_proc.p_flag = 0;

    // Initialize mib, which tells sysctl the info we want, in this case
    // we're looking for information about a specific process ID.

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    // Call sysctl.

    size = sizeof(info);
    junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
    assert(junk == 0);

    // We're being debugged if the P_TRACED flag is set.

    return ( (info.kp_proc.p_flag & P_TRACED) != 0 );
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
