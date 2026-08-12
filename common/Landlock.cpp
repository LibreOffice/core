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

const uint64_t AllowedToRead =
    LANDLOCK_ACCESS_FS_READ_FILE |
    LANDLOCK_ACCESS_FS_READ_DIR;

const uint64_t AllowedToWrite =
    LANDLOCK_ACCESS_FS_WRITE_FILE;

const uint64_t AllowedToWriteDir =
    LANDLOCK_ACCESS_FS_REMOVE_DIR |
    LANDLOCK_ACCESS_FS_REMOVE_FILE |
    LANDLOCK_ACCESS_FS_MAKE_DIR |
    LANDLOCK_ACCESS_FS_MAKE_REG |
    LANDLOCK_ACCESS_FS_MAKE_SYM;

/// Grant access beneath the permission's path. A path that does not exist is skipped.
bool addPerm(int rulesetFd, const Permission& perm)
{
    struct landlock_path_beneath_attr subPath = {};

    subPath.allowed_access = AllowedToRead;

    switch (perm._access)
    {
        case Access::ReadOnly:
        case Access::ReadOnlyDir:
            break;
        case Access::ReadWrite:
            subPath.allowed_access |= AllowedToWrite;
            break;
        case Access::ReadWriteDir:
            subPath.allowed_access |= AllowedToWrite | AllowedToWriteDir;
            break;
    }

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

} // anonymous namespace

bool isSupported()
{
    static int abi = landlockCreateRuleset(nullptr, 0, LANDLOCK_CREATE_RULESET_VERSION);
    return abi >= 1;
}

bool lock(const std::vector<Permission>& perms)
{
    if (!isSupported())
    {
        LOG_SYS("Landlock not present or enabled");
        return false;
    }

    // we work nicely with the oldest abi anyway

    struct landlock_ruleset_attr attr = {};

    attr.handled_access_fs =
        LANDLOCK_ACCESS_FS_EXECUTE |
        LANDLOCK_ACCESS_FS_WRITE_FILE |
        LANDLOCK_ACCESS_FS_READ_FILE |
        LANDLOCK_ACCESS_FS_READ_DIR |
        LANDLOCK_ACCESS_FS_REMOVE_DIR |
        LANDLOCK_ACCESS_FS_REMOVE_FILE |
        LANDLOCK_ACCESS_FS_MAKE_DIR |
        LANDLOCK_ACCESS_FS_MAKE_REG |
        LANDLOCK_ACCESS_FS_MAKE_SYM;

    const int rulesetFd = landlockCreateRuleset(&attr, sizeof(attr), 0);
    if (rulesetFd < 0)
    {
        LOG_SYS("Landlock can't create ruleset");
        return false;
    }

    bool success = true;

    // we allow access to already system protected file paths
    success = success && addPerm(rulesetFd, Permission("/etc", Access::ReadOnlyDir));

    success = success && addPerm(rulesetFd, Permission("/usr", Access::ReadOnlyDir));
    success = success && addPerm(rulesetFd, Permission("/lib", Access::ReadOnlyDir));
    success = success && addPerm(rulesetFd, Permission("/lib64", Access::ReadOnlyDir));
    success = success && addPerm(rulesetFd, Permission("/nix", Access::ReadOnlyDir));

    // fonts and fontconfig cache
    success = success && addPerm(rulesetFd, Permission("/var", Access::ReadOnlyDir));

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
