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
 * Landlock offers un-privileged restrictions of access to the file-system
 * to lock-down the environment, and to stop document processes seeing each
 * other. While stat() etc. still function, document jails include hard
 * random path elements to make them un-guessable.
 */

#include <config.h>

#include "Landlock.hpp"

#include <common/Log.hpp>

#include <cstdlib>

#if defined(__linux__) && __has_include(<linux/landlock.h>)
#define HAVE_LANDLOCK 1
#else
#define HAVE_LANDLOCK 0
#endif

#if HAVE_LANDLOCK
#include <cerrno>
#include <fcntl.h>
#include <glob.h>
#include <linux/landlock.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace Landlock {

#if !HAVE_LANDLOCK

bool isSupported()
{
    return false;
}

bool allowsCrossDirectoryRename()
{
    return true;
}

bool restrictsTruncate()
{
    return false;
}

bool lock(const std::vector<Permission>& /*perms*/)
{
    return false;
}

#else // HAVE_LANDLOCK

namespace {

// The C library has no wrappers for the landlock system calls, so call them directly.

#ifndef __NR_landlock_create_ruleset
#define __NR_landlock_create_ruleset 444
#endif
#ifndef __NR_landlock_add_rule
#define __NR_landlock_add_rule 445
#endif
#ifndef __NR_landlock_restrict_self
#define __NR_landlock_restrict_self 446
#endif

#ifndef LANDLOCK_ACCESS_FS_REFER
#define LANDLOCK_ACCESS_FS_REFER (1ULL << 13)
#endif
#ifndef LANDLOCK_ACCESS_FS_TRUNCATE
#define LANDLOCK_ACCESS_FS_TRUNCATE (1ULL << 14)
#endif

// ABI version that has LANDLOCK_ACCESS_FS_REFER to allow moving file to another directory
constexpr int ReferAbi = 2;

// ABI version that has LANDLOCK_ACCESS_FS_TRUNCATE - was unrestricted previously
constexpr int TruncateAbi = 3;

int landlockCreateRuleset(const struct landlock_ruleset_attr* attr, size_t size, uint32_t flags)
{
    return syscall(__NR_landlock_create_ruleset, attr, size, flags);
}

int landlockAddRule(int rulesetFd, enum landlock_rule_type ruleType, const void* ruleAttr,
                    uint32_t flags)
{
    return syscall(__NR_landlock_add_rule, rulesetFd, ruleType, ruleAttr, flags);
}

int landlockRestrictSelf(int rulesetFd, uint32_t flags)
{
    return syscall(__NR_landlock_restrict_self, rulesetFd, flags);
}

// Landlock ABI version, if not supported will be negative
int getAbi()
{
    static int abi = landlockCreateRuleset(nullptr, 0, LANDLOCK_CREATE_RULESET_VERSION);
    return abi;
}

// Landlock will only restrict operations that are added here
uint64_t getHandledAccess()
{
    uint64_t handled =
        LANDLOCK_ACCESS_FS_EXECUTE |
        LANDLOCK_ACCESS_FS_WRITE_FILE |
        LANDLOCK_ACCESS_FS_READ_FILE |
        LANDLOCK_ACCESS_FS_READ_DIR |
        LANDLOCK_ACCESS_FS_REMOVE_DIR |
        LANDLOCK_ACCESS_FS_REMOVE_FILE |
        LANDLOCK_ACCESS_FS_MAKE_CHAR |
        LANDLOCK_ACCESS_FS_MAKE_DIR |
        LANDLOCK_ACCESS_FS_MAKE_FIFO |
        LANDLOCK_ACCESS_FS_MAKE_REG |
        LANDLOCK_ACCESS_FS_MAKE_BLOCK |
        LANDLOCK_ACCESS_FS_MAKE_SOCK |
        LANDLOCK_ACCESS_FS_MAKE_SYM;

    if (getAbi() >= ReferAbi)
        handled |= LANDLOCK_ACCESS_FS_REFER;

    if (getAbi() >= TruncateAbi)
        handled |= LANDLOCK_ACCESS_FS_TRUNCATE;

    return handled;
}

const uint64_t AllowedToReadFile =
    LANDLOCK_ACCESS_FS_READ_FILE;

const uint64_t AllowedToRead =
    AllowedToReadFile |
    LANDLOCK_ACCESS_FS_READ_DIR;

const uint64_t AllowedToWrite =
    LANDLOCK_ACCESS_FS_WRITE_FILE |
    LANDLOCK_ACCESS_FS_TRUNCATE;

const uint64_t AllowedToWriteDir =
    LANDLOCK_ACCESS_FS_REMOVE_DIR |
    LANDLOCK_ACCESS_FS_REMOVE_FILE |
    LANDLOCK_ACCESS_FS_MAKE_DIR |
    LANDLOCK_ACCESS_FS_MAKE_REG |
    LANDLOCK_ACCESS_FS_MAKE_SYM |
    // Moving a file to another directory needs this access on both the source
    // and the target directory.
    LANDLOCK_ACCESS_FS_REFER;

/// Grant access beneath the permission's path. A path that does not exist is skipped.
bool addPerm(int rulesetFd, const Permission& perm)
{
    struct landlock_path_beneath_attr subPath = {};

    switch (perm._access)
    {
        case Access::ReadOnly:
            subPath.allowed_access = AllowedToReadFile;
            break;
        case Access::ReadOnlyDir:
            subPath.allowed_access = AllowedToRead;
            break;
        case Access::ReadWrite:
            subPath.allowed_access = AllowedToReadFile | AllowedToWrite;
            break;
        case Access::ReadWriteDir:
            subPath.allowed_access = AllowedToRead | AllowedToWrite | AllowedToWriteDir;
            break;
    }

    // restrict it to what the ABI supports
    subPath.allowed_access &= getHandledAccess();

    // opening with O_PATH so for symbolic links the rule attaches to the target file, not the link
    subPath.parent_fd = open(perm._path.c_str(), O_PATH | O_CLOEXEC);
    if (subPath.parent_fd < 0)
    {
        if (errno == ENOENT)
        {
            LOG_DBG("Landlock: skipped missing path '" << perm._path << "'");
            return true;
        }
        LOG_SYS("Landlock: failed to open '" << perm._path << "'");
        return false;
    }

    bool success = true;
    if (landlockAddRule(rulesetFd, LANDLOCK_RULE_PATH_BENEATH, &subPath, 0) != 0)
    {
        LOG_SYS("Landlock: failed to add '" << perm._path << "' to the ruleset with access "
                                            << subPath.allowed_access);
        success = false;
    }
    close(subPath.parent_fd);
    return success;
}

/// Grant read access to each file a wildcard pattern matches. A pattern that matches nothing is skipped.
bool addPermGlobReadOnly(int rulesetFd, const char* pattern)
{
    glob_t matches = {};

    const int globResult = glob(pattern, GLOB_NOSORT, nullptr, &matches);
    if (globResult == GLOB_NOMATCH)
    {
        globfree(&matches);
        return true;
    }

    if (globResult != 0)
    {
        LOG_ERR("Landlock: failed to expand '" << pattern << "', error " << globResult);
        globfree(&matches);
        return false;
    }

    bool success = true;
    for (size_t i = 0; i < matches.gl_pathc; ++i)
        success = success && addPerm(rulesetFd, Permission(matches.gl_pathv[i], Access::ReadOnly));

    globfree(&matches);
    return success;
}

} // anonymous namespace

bool isSupported()
{
    return getAbi() >= 1;
}

bool allowsCrossDirectoryRename()
{
    return getAbi() >= ReferAbi;
}

bool restrictsTruncate()
{
    return getAbi() >= TruncateAbi;
}

bool lock(const std::vector<Permission>& perms)
{
    if (!isSupported())
    {
        LOG_SYS("Landlock not present or enabled");
        return false;
    }

    // we work nicely with the oldest abi, and use some newer features when available

    struct landlock_ruleset_attr attr = {};

    attr.handled_access_fs = getHandledAccess();

    LOG_DBG("Landlock: abi " << getAbi() << ", handling access " << std::hex
                             << attr.handled_access_fs << std::dec);

    const int rulesetFd = landlockCreateRuleset(&attr, sizeof(attr), 0);
    if (rulesetFd < 0)
    {
        LOG_SYS("Landlock can't create ruleset");
        return false;
    }

    bool success = true;

    // Try to use same list of files as coolwsd-systemplate-setup.
    // (non-existent files will be skipped)
    for (const char* file : {
            "/etc/ld.so.cache",
            "/etc/ld.so.conf",
            "/etc/hosts",
            "/etc/resolv.conf",
            "/etc/passwd",
            "/etc/group",
            "/etc/host.conf",
            "/etc/nsswitch.conf",
            // the certificate bundle, in each of the places a distribution keeps it
            "/etc/pki/tls/certs/ca-bundle.crt",
            "/etc/pki/tls/certs/ca-bundle.trust.crt",
            "/etc/ssl/certs/ca-certificates.crt",
            "/var/lib/ca-certificates/ca-bundle.pem",
        })
    {
        success = success && addPerm(rulesetFd, Permission(file, Access::ReadOnly));
    }

    success = success && addPerm(rulesetFd, Permission("/etc/ld.so.conf.d", Access::ReadOnlyDir));
    success = success && addPerm(rulesetFd, Permission("/etc/fonts", Access::ReadOnlyDir));
    success =
        success && addPerm(rulesetFd, Permission("/var/cache/fontconfig", Access::ReadOnlyDir));

    // symlinks from /etc point there
    success = success && addPerm(rulesetFd, Permission("/usr/share/fontconfig", Access::ReadOnlyDir));

    // ancient bitmap fonts are not filtered out, presumably that was just done to
    // improve performance of copying
    success = success && addPerm(rulesetFd, Permission("/usr/share/fonts", Access::ReadOnlyDir));
    success = success && addPerm(rulesetFd, Permission("/usr/share/ghostscript/fonts", Access::ReadOnlyDir));
    success = success && addPerm(rulesetFd, Permission("/usr/local/share/fonts", Access::ReadOnlyDir));

#if ENABLE_DEBUG
    auto const pHome = getenv("HOME");
    if (pHome) {
        const std::string home{pHome};
        success = success && addPerm(rulesetFd, Permission(home + "/.fonts", Access::ReadOnlyDir));
    }
#endif

    for (const char* pattern : {
            "/lib/ld-*",
            "/lib64/ld-*",
            "/lib/libnss_*",
            "/lib64/libnss_*",
            "/lib/*/libnss_*",
            "/lib/libresolv*",
            "/lib64/libresolv*",
            "/lib/*/libresolv*",
        })
    {
        success = success && addPermGlobReadOnly(rulesetFd, pattern);
    }

    success = success && addPerm(rulesetFd, Permission("/nix/store", Access::ReadOnlyDir));

    // NB. no /dev device nodes or /sys or /proc pieces are needed - we
    // patch libraries internally to avoid needing these.

    for (const auto& perm : perms)
        success = success && addPerm(rulesetFd, perm);

    // The kernel only lets an un-privileged process restrict itself once the
    // no-new-privs flag is set, so set that first.
    if (success && prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0)
    {
        LOG_SYS("Cannot turn off acquisition of new privileges for us & children");
        success = false;
    }

    if (success && landlockRestrictSelf(rulesetFd, 0) != 0)
    {
        LOG_SYS("Failed to enforce landlock rules");
        success = false;
    }
    close(rulesetFd);

    return success;
}

#endif // HAVE_LANDLOCK

void setAllowedPaths(const std::vector<Permission>& perms)
{
    std::string envVar;
    for (const auto& perm : perms)
    {
        switch (perm._access)
        {
            case Access::ReadOnly:
            case Access::ReadOnlyDir:
                envVar += ":r:";
                break;
            case Access::ReadWrite:
            case Access::ReadWriteDir:
                envVar += ":w:";
                break;
        }
        envVar += perm._path;
    }
    ::setenv("SAL_ALLOWED_PATHS", envVar.c_str(), 1);
}

} // namespace Landlock

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
