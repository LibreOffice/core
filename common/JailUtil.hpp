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

#include <string>
#include <string_view>
#include <sys/types.h>

namespace JailUtil
{

/// Holds one directory per jail: cool-<jailId> for the jail's own tmp, and systemplate-<jailId>
/// carrying an up-to-date copy of the system template's etc directory when the installed one is
/// out of date.
const std::string CHILDROOT_TMP_PATH = "/tmp";

/// Files uploaded by users are stored in this sub-directory of child-root.
const std::string CHILDROOT_TMP_INCOMING_PATH = "/tmp/incoming";

/// The preset configuration files a configuration id carries, shared by every jail that names it.
const std::string CHILDROOT_TMP_SHARED_PRESETS_PATH = "/tmp/sharedpresets";

/// The LO installation directory with jail.
const std::string LO_JAIL_SUBPATH = "lo";

/// The directory within a jail that holds the shared presets of the configuration the jail
/// serves. It has one subdirectory per preset group.
const std::string PRESETS_JAIL_SUBPATH = "presets";

#ifdef __linux__

/** Linux user/mount namespaces

    There cannot be other threads running when calling these namespace
    functions or they will fail.

    These user namespaces stack, so each call to enter..NS creates another
    namespace inside the current one. In practice you can't return to a higher
    user namespace level. man 7 user_namespaces
 */

/// Try to put this process into its own user and mount namespace and
/// map uid/gid to root within that namespace to allow mounting
bool enterMountingNS(uid_t uid, gid_t gid);

/// Try to put this process into its own user namespace and
/// map root to uid/gid within that namespace.
bool enterUserNS(uid_t uid, gid_t gid);

/// The seccomp mode of a process, read from the text of its /proc/<pid>/status. Zero when no
/// filter is installed, and when the field is absent or unreadable.
int seccompModeFromStatus(std::string_view procSelfStatus);

/// One sentence naming which setting forbids a new user namespace, empty when none of the three
/// explains it. The first two arguments are the text of /proc/sys/user/max_user_namespaces and of
/// /proc/sys/kernel/apparmor_restrict_unprivileged_userns, each empty on a host without it.
std::string explainNamespaceRefusal(std::string_view maxUserNamespaces,
                                    std::string_view apparmorRestrictUserns, int seccompMode);

#endif // __linux__

/// Bind mount a jail directory.
bool bind(const std::string& source, const std::string& target);

/// Remount a bound mount point as readonly.
bool remountReadonly(const std::string& source, const std::string& target);

/// Marks a jail as having been copied instead of mounted.
void markJailCopied(const std::string& root);

/// Returns true iff the jail in question was copied and not mounted.
bool isJailCopied(const std::string& root);

/// Remove the jail directory and all its contents.
bool tryRemoveJail(const std::string& root);

/// Remove all jails.
void cleanupJails(const std::string& jailRoot);

/// Creates the jail directory path recursively.
void createJailPath(const std::string& path);

/// Setup the Child-Root directory.
void setupChildRoot(bool bindMount, const std::string& jailRoot, const std::string& sysTemplate);

/// Enable bind-mounting in this process.
void enableBindMounting();

/// Disable bind-mounting in this process.
void disableBindMounting();

/// Returns true iff bind-mounting is enabled in this process.
bool isBindMountingEnabled();

/// Flag that bind-mounting is configured.
void enableBindMountingConfigured();
/// Unflag that bind-mounting is configured.
void disableBindMountingConfigured();

/// Returns true iff bind-mounting is configured in coolwsd.xml.
bool isBindMountingConfigured();

/// Enable namespace-mounting in this process.
void enableMountNamespaces();

/// Disable namespace-mounting in this process.
void disableMountNamespaces();

/// Returns true iff namespace-mounting is enabled in this process.
bool isMountNamespacesEnabled();

#if ENABLE_CHILDROOTS
namespace SysTemplate
{
/// Setup links for /dev/random and /dev/urandom in systemplate.
void setupRandomDeviceLinks(const std::string& root);

/// Setup the dynamic files within the sysTemplate by either
/// copying or linking. See updateJail_DynamicFilesInSysTemplate.
/// If the dynamic files need updating and systemplate is read-only,
/// this will fail and mark files for copying.
void setupDynamicFiles(const std::string& sysTemplate);

/// Update the dynamic files within the sysTemplate before each child fork.
/// Returns false on failure.
bool updateDynamicFiles(const std::string& sysTemplate);

} // namespace SysTemplate
#endif // ENABLE_CHILDROOTS

} // end namespace JailUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
