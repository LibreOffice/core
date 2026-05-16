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
 * Jail filesystem setup and management utilities.
 * Functions: setupJail(), cleanupJails(), loopMountroot()
 */

#include <config.h>

#include "JailUtil.hpp"

#include <common/FileUtil.hpp>
#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/SigUtil.hpp>
#include <common/Util.hpp>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sysexits.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/sysmacros.h>
#endif

extern int domount(int argc, const char* const* argv);

namespace JailUtil
{

namespace
{

static const std::string CoolTestMountpoint = "cool_test_mount";

#ifdef __linux__
static void setdeny()
{
    std::ofstream of("/proc/self/setgroups");
    of << "deny";
}

static void mapuser(uid_t origuid, uid_t newuid, gid_t origgid, gid_t newgid)
{
    {
        std::ofstream of("/proc/self/uid_map");
        of << newuid << " " << origuid << " 1";
    }

    {
        std::ofstream of("/proc/self/gid_map");
        of << newgid << " " << origgid << " 1";
    }
}
#endif // __linux__

} // namespace

#ifdef __linux__
bool enterMountingNS(uid_t uid, gid_t gid)
{
    // Put this process into its own user and mount namespace.
    // Note: Having multiple threads at unshare time is a known source of failure.
    if (unshare(CLONE_NEWUSER) != 0)
    {
        LOG_SYS("enterMountingNS, CLONE_NEWUSER unshare failed");
        return false;
    }

    setdeny();

    // Map this user as the root user of the new namespace
    mapuser(uid, 0, gid, 0);

    if (unshare(CLONE_NEWNS) != 0)
    {
        LOG_SYS("enterMountingNS, CLONE_NEWNS unshare failed");
        return false;
    }

    // Do not propagate any mounts from this new namespace to the system.
    if (mount("none", "/", nullptr, MS_REC | MS_PRIVATE, nullptr) != 0)
    {
        LOG_SYS("enterMountingNS, root mount failed");
        return false;
    }

    return true;
}

bool enterUserNS(uid_t uid, gid_t gid)
{
    if (unshare(CLONE_NEWUSER) != 0)
    {
        // having multiple threads is a source of failure f.e.
        LOG_SYS("enterMountingNS, unshare failed");
        return false;
    }

    // undo map of this user to root
    mapuser(0, uid, 0, gid);

    assert(geteuid() == uid);
    assert(getegid() == gid);

    return true;
}
#endif // __linux__

namespace
{

static bool coolmount(const std::string& arg, std::string source, std::string target,
                      bool silent = false)
{
    source = Util::rtrim(source, '/');
    target = Util::rtrim(target, '/');

    if (isMountNamespacesEnabled())
    {
        const char *argv[5];
        argv[0] = "notcoolmount";
        int argc = 1;
        if (!arg.empty())
            argv[argc++] = arg.c_str();
        if (silent)
            argv[argc++] = "-s";
        if (!source.empty())
            argv[argc++] = source.c_str();
        if (!target.empty())
            argv[argc++] = target.c_str();
        return domount(argc, argv) == EX_OK;
    }

    const std::string cmd = Poco::Path(Util::getApplicationPath(), "coolmount").toString() + ' '
                            + arg + (silent ? " -s" : " ") + source + ' ' + target;
    LOG_TRC("Executing coolmount command: " << cmd);
    return !system(cmd.c_str());
}
} // namespace

bool bind(const std::string& source, const std::string& target)
{
    LOG_DBG("Mounting [" << source << "] -> [" << target << ']');
    try
    {
        Poco::File(target).createDirectory();
        const bool res = coolmount("-b", source, target);
        if (res)
            LOG_TRC("Bind-mounted [" << source << "] -> [" << target << ']');
        else
            LOG_ERR("Failed to bind-mount [" << source << "] -> [" << target << ']');
        return res;
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Failed to mount [" << source << "] -> [" << target << "]: " << exc.what());
    }

    return false;
}

bool remountReadonly(const std::string& source, const std::string& target)
{
    LOG_DBG("Remounting [" << source << "] -> [" << target << ']');
    try
    {
        Poco::File(target).createDirectory();
        const bool res = coolmount("-r", source, target);
        if (res)
            LOG_TRC("Mounted [" << source << "] -> [" << target << "] readonly");
        else
            LOG_ERR("Failed to mount [" << source << "] -> [" << target << "] readonly");
        return res;
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Failed to remount [" << source << "] -> [" << target << "]: " << exc.what());
    }

    return false;
}

namespace
{

/// Unmount a bind-mounted jail directory.
static bool unmount(const std::string& target, bool silent = false)
{
    LOG_DBG("Unmounting [" << target << ']');
    const bool res = coolmount("-u", "", target, silent);
    if (res)
        LOG_TRC("Unmounted [" << target << "] successfully.");
    else
    {
        // If bind-mounting is enabled, noisily log failures.
        // Otherwise, it's a cleanup attempt of earlier mounts,
        // which may be left-over and now the config has changed.
        // This happens more often in dev labs than in prod.
        if (JailUtil::isBindMountingEnabled() && !silent)
            LOG_WRN("Failed to unmount [" << target << ']');
        else
            LOG_DBG("Failed to unmount [" << target << ']');
    }

    return res;
}
} // namespace

// This file signifies that we copied instead of mounted.
// NOTE: jail cleanup helpers are called from forkit and
// coolwsd, and they may have bind-mounting enabled, but the
// kit could have had it removed when falling back to copying.
// In such cases, we cannot safely know whether the jail was
// copied or not, since the bind envar will be present and
// assuming it was mounted, would leak them.
// Alternatively, if we remove the files when mounted
// we could destroy systemplate if remounting read-only had
// failed (and it wasn't owned by root).
constexpr const char* COPIED_JAIL_MARKER_FILE = "delete.me";

void markJailCopied(const std::string& root)
{
#if ENABLE_CHILDROOTS
    // The reason we should be able to create this file
    // is because the jail must be writable.
    // Failing this will cause an exception, signaling an error.
    Poco::File(root + '/' + COPIED_JAIL_MARKER_FILE).createFile();
#endif
}

#if ENABLE_CHILDROOTS
bool isJailCopied(const std::string& root)
{
    // If the marker file exists, the jail was copied.
    FileUtil::Stat delFileStat(root + '/' + COPIED_JAIL_MARKER_FILE);
    return delFileStat.exists();
}

namespace
{

static bool safeRemoveDir(const std::string& path)
{
    // Always unmount, just in case.
    unmount(path, /*silent=*/true);

    // Regardless of the bind flag, check if the jail is marked as copied.
    const bool copied = isJailCopied(path);

    // We must be empty if we had mounted.
    if (!copied && JailUtil::isBindMountingEnabled() && !FileUtil::isEmptyDirectory(path))
    {
        LOG_WRN("Path [" << path << "] is not empty. Will not remove it.");
        return false;
    }

    // Recursively remove if link/copied.
    if (copied)
    {
        //FIXME: do not delete the 'copied' marker until the very end.
        FileUtil::removeFile(path, /*recursive=*/true);
    }
    else
    {
        FileUtil::removeEmptyDirTree(path);
    }

    return true;
}

void removeAuxFolders(const std::string &root)
{
    FileUtil::removeFile(Poco::Path(root, "tmp").toString(), true);
    FileUtil::removeFile(Poco::Path(root, "linkable").toString(), true);
}
#endif

/*
    The tmp dir of a path/<jailid>/tmp is mounted from (or linked to) a
    path/tmp/cool-<jailid> dir. In a mount namespace case the existence
    of path/<jailid>/tmp is not visible to the parent process so its
    contents cannot be removed via the path/<jailid>/tmp view, and
    in any case the path/<jailid>/tmp should be removed.
*/
void removeAssocTmpOfJail(const std::string &root)
{
#if ENABLE_CHILDROOTS
    Poco::Path jailPath(root);
    jailPath.makeDirectory();
    const std::string jailId = jailPath[jailPath.depth() - 1];

    jailPath.popDirectory();
    jailPath.pushDirectory("tmp");

    Poco::Path coolTmpPath(jailPath);
    coolTmpPath.pushDirectory(std::string("cool-") + jailId);
    FileUtil::removeFile(coolTmpPath.toString(), true);

    // Also remove the per-jail systemplate copy created when
    // sysTemplateIncomplete is true (read-only systemplate).
    Poco::Path sysTemplatePath(jailPath);
    sysTemplatePath.pushDirectory(std::string("systemplate-") + jailId);
    FileUtil::removeFile(sysTemplatePath.toString(), true);
#endif
}

} // namespace

bool tryRemoveJail(const std::string& root)
{
#if ENABLE_CHILDROOTS
    const bool emptyJail = FileUtil::isEmptyDirectory(root);
    if (!emptyJail && !FileUtil::Stat(root + '/' + LO_JAIL_SUBPATH).exists())
        return false; // not a jail.

    LOG_TRC("Do remove of jail [" << root << ']');

    if (!emptyJail)
    {
        // Unmount the tmp directory. Don't care if we fail.
        const std::string tmpPath = Poco::Path(root, "tmp").toString();
#ifdef __FreeBSD__
        unmount(tmpPath + "/dev");
#endif
        FileUtil::removeFile(tmpPath, true); // Delete tmp contents with prejudice.
        unmount(tmpPath);

        // Unmount the loTemplate directory.
        //FIXME: technically, the loTemplate directory may have any name.
        unmount(Poco::Path(root, "lo").toString());

        // Unmount the test-mount directory too.
        const std::string testMountPath = Poco::Path(root, CoolTestMountpoint).toString();
        if (FileUtil::Stat(testMountPath).exists())
            unmount(testMountPath);
    }

    // Unmount/delete the jail (sysTemplate).
    safeRemoveDir(root);

    removeAssocTmpOfJail(root);
#endif

    return true;
}

/// This cleans up the jails directories.
/// Note that we assume the templates are mounted
/// and we unmount first. This is critical, because
/// otherwise when mounting is disabled we may
/// inadvertently delete the contents of the mount-points.
void cleanupJails(const std::string& root)
{
#if ENABLE_CHILDROOTS
    LOG_INF("Cleaning up childroot directory [" << root << "].");

    FileUtil::Stat stRoot(root);
    if (!stRoot.exists() || !stRoot.isDirectory())
    {
        LOG_TRC("Directory [" << root << "] is not a jail directory or doesn't exist.");
        return;
    }

    std::vector<std::string> jails;
    Poco::File(root).list(jails);

    // legacy jails at the top-level
    for (const auto& jail : jails)
    {
        std::string childDir = Poco::Path(root, jail).toString();
        FileUtil::Stat stChild(childDir);
        if (stChild.exists() && !stChild.isLink() && stChild.isDirectory())
        {
            // Modern jails should look like this:
            //   jails/<coolwsd-pid>-<random>/<random>/
            size_t pidSepPos = jail.find('-');
            if (pidSepPos != std::string::npos)
            {
                bool skip = false;
                const std::string_view pidStr = std::string_view(jail).substr(0, pidSepPos);
                const auto [pid, success] = NumUtil::i32FromString(pidStr);
                if (success && pid > 1)
                {
                    LOG_TRC("Checking pid for jail " << pid << " " << root);
                    if (pid != getpid() && ::kill(pid, 0) == 0)
                    {
                        LOG_TRC("Skipping cleaning jails directory for running coolwsd with pid " << pid);
                        skip = true;
                    }
                }
                else
                {
                    // Problematic - may delete a jail that is not ours then ...
                    LOG_WRN("Exception parsing pid '" << pidStr << "' from '" << jail << "'");
                }

                if (!skip)
                {
                    std::vector<std::string> newJails;
                    Poco::File(childDir).list(newJails);

                    // legacy jails at the top-level
                    for (const auto& newJail : newJails)
                    {
                        const std::string path = Poco::Path(childDir, newJail).toString();
                        if (newJail == CoolTestMountpoint)
                            safeRemoveDir(path);
                        else
                            tryRemoveJail(path);
                    }

                    // top level linkable and tmp mount point.
                    removeAuxFolders(childDir);

                    // top level per-coolwsd jails directory.
                    safeRemoveDir(childDir);
                }
            }
            // Remove legacy things that look like jails
            else if (tryRemoveJail(childDir))
            {
                LOG_WRN_ONCE("Cleaned legacy jail without pid prefix after upgrade " << childDir);
            }
            // else legacy tmp or linkable
        }
    }

    // Cleanup legacy top-level 'tmp' and 'linkable' directories if empty
    removeAuxFolders(root);

    // Cleanup top-level 'jails' directory if empty
    if (FileUtil::isEmptyDirectory(root))
        safeRemoveDir(root);
    else
        LOG_WRN("Jails root directory [" << root << "] is not empty. Will not remove it.");
#endif
}

void createJailPath(const std::string& path)
{
#if ENABLE_CHILDROOTS
    LOG_INF("Creating jail path (if missing): " << path);
    Poco::File(path).createDirectories();
    if (chmod(path.c_str(), S_IXUSR | S_IWUSR | S_IRUSR) != 0)
        LOG_WRN_SYS("chmod(\"" << path << "\") failed");
#endif
}

void setupChildRoot(bool bindMount, const std::string& childRoot, const std::string& sysTemplate)
{
#if ENABLE_CHILDROOTS
    // Start with a clean slate.
    cleanupJails(childRoot);

    createJailPath(childRoot + CHILDROOT_TMP_INCOMING_PATH + "/fonts");
    createJailPath(childRoot + CHILDROOT_TMP_SHARED_PRESETS_PATH);

    disableBindMounting(); // Clear to avoid surprises.

    // Try to enable bind-mounting if requested (via config).
    if (bindMount)
    {
        // Test mounting to verify it actually works,
        // as it might not function in some systems.
        const std::string target = Poco::Path(childRoot, CoolTestMountpoint).toString();

        // Make sure that we can mount, remount and unmount before enabling bind-mounting.
        const bool bindWorked = bind(sysTemplate, target);
        const bool remountWorked = bindWorked && remountReadonly(sysTemplate, target);
        const bool unmountWorked = bindWorked && unmount(target);
        if (remountWorked && unmountWorked)
        {
            enableBindMounting();
            safeRemoveDir(target);
            LOG_INF("Enabling Bind-Mounting of jail contents for better performance per "
                    "mount_jail_tree config in coolwsd.xml.");
        }
        else
            LOG_ERR("Bind-Mounting fails and will be disabled for this run. To disable permanently "
                    "set mount_jail_tree config entry in coolwsd.xml to false.");
    }
    else
        LOG_INF("Disabling Bind-Mounting of jail contents per "
                "mount_jail_tree config in coolwsd.xml.");
#endif
}

/// The envar name used to control bind-mounting of systemplate/jails.
constexpr const char* BIND_MOUNTING_ENVAR_NAME = "COOL_BIND_MOUNT";
/// The envar name used to signal whether bind-mounting is configured.
constexpr const char* BIND_MOUNTING_CONFIGURED_ENVAR_NAME = "COOL_BIND_MOUNT_CONFIGURED";

void enableBindMounting()
{
    // Set the envar to enable.
    setenv(BIND_MOUNTING_ENVAR_NAME, "1", 1);
}

void disableBindMounting()
{
    // Remove the envar to disable.
    unsetenv(BIND_MOUNTING_ENVAR_NAME);
}

bool isBindMountingEnabled()
{
    // Check if we have a valid envar set.
    return std::getenv(BIND_MOUNTING_ENVAR_NAME) != nullptr;
}

void enableBindMountingConfigured()
{
    // Set the envar to enable.
    setenv(BIND_MOUNTING_CONFIGURED_ENVAR_NAME, "1", 1);
}

void disableBindMountingConfigured()
{
    // Remove the envar to disable.
    unsetenv(BIND_MOUNTING_CONFIGURED_ENVAR_NAME);
}

bool isBindMountingConfigured()
{
    // Check if we have a valid envar set.
    return std::getenv(BIND_MOUNTING_CONFIGURED_ENVAR_NAME) != nullptr;
}

constexpr const char* NAMESPACE_MOUNTING_ENVAR_NAME = "COOL_NAMESPACE_MOUNT";

void enableMountNamespaces()
{
    // Set the envar to enable.
    setenv(NAMESPACE_MOUNTING_ENVAR_NAME, "1", 1);
}

void disableMountNamespaces()
{
    // Remove the envar to enable.
    unsetenv(NAMESPACE_MOUNTING_ENVAR_NAME);
}

bool isMountNamespacesEnabled()
{
    // Check if we have a valid envar set.
    return std::getenv(NAMESPACE_MOUNTING_ENVAR_NAME) != nullptr;
}


#if ENABLE_CHILDROOTS
namespace SysTemplate
{
/// The network and other system files we need to keep up-to-date in jails.
/// These must be up-to-date, as they can change during
/// the long lifetime of our process. Also, it's unlikely
/// that systemplate will get re-generated after installation.
static const auto DynamicFilePaths
    = { "/etc/passwd", "/etc/group",
        "/etc/host.conf", "/etc/hosts",
        "/etc/nsswitch.conf", "/etc/resolv.conf" };

namespace
{
/// Copy (false) by default for KIT_IN_PROCESS.
static bool LinkDynamicFiles = false;

bool updateDynamicFilesImpl(const std::string& sysTemplate)
{
    LOG_INF("Updating systemplate dynamic files in [" << sysTemplate << ']');

    bool checkWritableSysTemplate = true;
    for (const auto& dynFilename : DynamicFilePaths)
    {
        if (!FileUtil::Stat(dynFilename).exists())
        {
            LOG_INF("Dynamic file [" << dynFilename
                                     << "] does not exist. Some functionality may be affected.");
            continue;
        }

        const std::string srcFilename = FileUtil::realpath(dynFilename);
        if (srcFilename != dynFilename)
        {
            LOG_TRC("Dynamic file [" << dynFilename << "] points to real path [" << srcFilename
                                     << "], which will be used instead.");
        }

        FileUtil::Stat srcStat(srcFilename);
        if (!srcStat.exists())
            continue;

        const std::string dstFilename = Poco::Path(sysTemplate, dynFilename).toString();
        FileUtil::Stat dstStat(dstFilename);

        // Is it outdated?
        if (FileUtil::Stat::isUpToDate(srcStat, srcFilename, dstStat, dstFilename))
        {
            LOG_TRC("File [" << dstFilename << "] is already up-to-date.");
            continue;
        }

        if (checkWritableSysTemplate && !FileUtil::isWritable(sysTemplate))
        {
            LinkDynamicFiles = false;
            LOG_WRN("The systemplate directory ["
                    << sysTemplate << "] is read-only, and at least [" << dstFilename
                    << "] is out-of-date. Will have to clone dynamic elements of "
                    << "systemplate to the jails. To restore optimal performance, "
                    << "make sure the files in [" << sysTemplate << "/etc] "
                    << "are up-to-date.");
            return false;
        }

        checkWritableSysTemplate = false; // We've checked and is writable.

        LOG_INF("File [" << dstFilename << "] needs to be updated.");
        if (LinkDynamicFiles)
        {
            LOG_INF("Linking [" << srcFilename << "] -> [" << dstFilename << "].");

            // Link or copy.
            if (link(srcFilename.c_str(), dstFilename.c_str()) == 0)
                continue;

            // Hard-linking failed, try symbolic linking.
            if (symlink(srcFilename.c_str(), dstFilename.c_str()) == 0)
                continue;

            const int linkerr = errno;

            // With parallel tests, another test might have linked already.
            FileUtil::Stat dstStat2(dstFilename);
            if (FileUtil::Stat::isUpToDate(dstStat2, dstFilename, srcStat, srcFilename))
            {
                LOG_INF("File [" << dstFilename << "] now seems to be up-to-date.");
                continue;
            }

            // Failed to link a file. Disable linking and copy instead.
            LOG_WRN_ERRNO(
                linkerr,
                "Failed to link ["
                    << srcFilename << "] -> [" << dstFilename
                    << "]. Will copy and disable linking dynamic system files in this run");
            LinkDynamicFiles = false;
        }

        // Linking failed, just copy.
        if (!LinkDynamicFiles)
        {
            LOG_INF("Copying [" << srcFilename << "] -> [" << dstFilename << ']');
            if (!FileUtil::copyAtomic(srcFilename, dstFilename, true))
            {
                FileUtil::Stat dstStat2(dstFilename); // Stat again.
                if (!FileUtil::Stat::isUpToDate(dstStat2, dstFilename, srcStat, srcFilename))
                {
                    return false; // No point in trying the remaining files.
                }
            }

            // Create the 'copied' file so we keep the files up-to-date.
            Poco::File(Poco::Path(sysTemplate, "etc/copied").toString()).createFile();
        }
    }

    return true;
}
} // namespace

void setupDynamicFiles(const std::string& sysTemplate)
{
    LOG_INF("Setting up systemplate dynamic files in [" << sysTemplate << "].");

    LinkDynamicFiles = true; // Prefer linking, unless it fails.

    const bool uptodate = updateDynamicFilesImpl(sysTemplate);
    if (!uptodate)
    {
        // Can't copy!
        LOG_WRN("Failed to update the dynamic files in ["
                << sysTemplate << "]. Will clone dynamic elements of systemplate to the jails.");
        LinkDynamicFiles = false;
    }

    FileUtil::Stat copiedFileStat(Poco::Path(sysTemplate, "etc/copied").toString());
    if (copiedFileStat.exists())
    {
        // At least one file is copied, we must check for changes before each jail setup.
        LinkDynamicFiles = false;
    }

    LOG_INF("Systemplate dynamic files in ["
            << sysTemplate << "] "
            << (LinkDynamicFiles ? "are linked and will remain" : "will be copied to keep them")
            << " up-to-date.");
}

bool updateDynamicFiles(const std::string& sysTemplate)
{
    // If the files are linked, they are always up-to-date.
    return LinkDynamicFiles ? true : updateDynamicFilesImpl(sysTemplate);
}

namespace
{

void setupRandomDeviceLink(const std::string& sysTemplate, const std::string& name)
{
    const std::string path = sysTemplate + "/dev/";
    try
    {
        // Create the path first.
        Poco::File(path).createDirectories();
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Failed to create [" << path << "]: " << ex.what());
        return;
    }

    const std::string linkpath = path + name;
    const std::string target = "../tmp/dev/" + name;
    LOG_DBG("Linking symbolically [" << linkpath << "] to [" << target << "].");

    const FileUtil::Stat stLink(linkpath, true); // The file is a link.
    if (stLink.exists())
    {
        if (!stLink.isLink())
            LOG_WRN("Random device link [" << linkpath << "] exists but isn't a link.");
        else
            LOG_TRC("Random device link [" << linkpath << "] already exists.");

        return;
    }

    if (symlink(target.c_str(), linkpath.c_str()) == -1)
    {
        LOG_SYS(
            "Failed to create symlink to ["
            << name << "] device at [" << target << "] pointing to source [" << linkpath
            << "]. Some features, such us password-protection and document-signing might not work");
    }
}
} // namespace

// The random devices are setup in two stages.
// This is the first stage, where we create symbolic links
// in sysTemplate/dev/[u]random pointing to ../tmp/dev/[u]random
// when we setup sysTemplate in forkit.
// In the second stage, during jail creation, we create the dev
// nodes in /tmp/dev/[u]random inside the jail chroot.
void setupRandomDeviceLinks(const std::string& sysTemplate)
{
    setupRandomDeviceLink(sysTemplate, "random");
    setupRandomDeviceLink(sysTemplate, "urandom");
}

} // namespace SysTemplate
#endif // ENABLE_CHILDROOTS

} // namespace JailUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
