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
 * The main entry point for the COKit process serving
 * a document editing session.
 */

#include <config.h>

#include "Kit.hpp"

#include <common/Anonymizer.hpp>
#include <common/Common.hpp>
#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/MobileApp.hpp>
#include <common/Png.hpp>
#include <common/Protocol.hpp>
#include <common/Rectangle.hpp>
#include <common/RenderTiles.hpp>
#include <common/Unit.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <kit/ChildSession.hpp>
#include <kit/KitHelper.hpp>
#include <kit/KitWebSocket.hpp>
#include <wsd/TileDesc.hpp>
#include <wsd/UserMessages.hpp>

#if !MOBILEAPP
#include <common/JailUtil.hpp>
#include <common/Seccomp.hpp>
#include <common/SigUtil.hpp>
#include <common/Syscall.hpp>
#include <common/TraceEvent.hpp>
#include <common/Watchdog.hpp>
#include <common/security.h>
#include <kit/BgSaveWatchDog.hpp>
#else // MOBILEAPP
#include <wsd/COOLWSD.hpp>
#ifndef IOS
#include <kit/SetupKitEnvironment.hpp>
#endif
#endif // MOBILEAPP

#include <csignal>
#include <limits>

#if !MOBILEAPP
#include <dlfcn.h>
#endif

#ifdef __linux__
#include <ftw.h>
#include <sys/vfs.h>
#include <linux/magic.h>
#include <sys/sysmacros.h>
#endif

#if HAVE_LIBCAP
#include <sys/capability.h>
#endif

#if defined(__FreeBSD__) || defined(MACOS) || (defined(__linux__) && !defined(__GLIBC__))
#include <ftw.h>
// FTW_CONTINUE, FTW_STOP, FTW_SKIP_SUBTREE, FTW_ACTIONRETVAL are glibc extensions
#define FTW_CONTINUE 0
#define FTW_STOP (-1)
#define FTW_SKIP_SUBTREE 0
#define FTW_ACTIONRETVAL 0
#endif

#ifndef _WIN32
#include <unistd.h>
#include <utime.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sysexits.h>
#endif

#include <atomic>
#include <cassert>
#include <chrono>
#include <climits>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

#define KIT_USE_UNSTABLE_API
#include <COKit/COKitInit.h>
#include <COKit/COKit.hxx>

#include <Poco/File.h>
#include <Poco/Exception.h>
#include <Poco/URI.h>

#ifdef QTAPP
#include <kit/SetupKitEnvironment.hpp>
#include <wsd/DocumentBroker.hpp>
#include <future>
#endif
#ifdef IOS
#include <ios.h>
#include <wsd/DocumentBroker.hpp>
#elif defined(MACOS) && MOBILEAPP
#include <macos.h>
#include <wsd/DocumentBroker.hpp>
#endif

#ifdef _WIN32
#include "windows.hpp"
#endif

using Poco::Exception;
using Poco::File;
using Poco::JSON::Object;
using Poco::JSON::Parser;

#ifndef BUILDING_TESTS
using Poco::Path;
#endif

using namespace COOLProtocol;
using JsonUtil::makePropertyValue;

extern "C" { void dump_kit_state(void); /* easy for gdb */ }

#if MOBILEAPP
extern std::map<std::string, std::shared_ptr<DocumentBroker>> DocBrokers;
extern std::mutex DocBrokersMutex;
#endif

#if !MOBILEAPP

// A Kit process hosts only a single document in its lifetime.
class Document;
static Document *singletonDocument = nullptr;
static std::unique_ptr<Util::ThreadCounter> threadCounter;
static std::unique_ptr<Util::FDCounter> fdCounter;

int getCurrentThreadCount()
{
    if (threadCounter)
        return threadCounter->count();
    return -1;
}

#endif

COKit* loKitPtr = nullptr;

static bool EnableWebsocketURP = false;
#if !MOBILEAPP
static int URPStartCount = 0;
#endif

bool isURPEnabled() { return EnableWebsocketURP; }

/// When chroot is enabled, this is blank as all
/// the paths inside the jail, relative to it's jail.
/// E.g. /tmp/user/docs/...
/// However, without chroot, the jail path is
/// absolute in the system root.
/// I.e. ChildRoot/JailId/tmp/user/docs/...
/// We need to know where the jail really is
/// because WSD doesn't know if chroot will succeed
/// or fail, but it assumes the document path to
/// be relative to the root of the jail (i.e. chroot
/// expected to succeed). If it fails, or when caps
/// are disabled, file paths would be relative to the
/// system root, not the jail.
static std::string JailRoot;

#if !MOBILEAPP
static int URPtoLoFDs[2] { -1, -1 };
static int URPfromLoFDs[2] { -1, -1 };
#endif

// Abnormally we get COKit events from another thread, which must be
// push safely into our main poll loop to process to keep all
// socket buffer & event processing in a single, thread.
static bool pushToMainThread(COKitCallback cb, int type, const char* p, void* data);

[[maybe_unused]]
static CokHookFunction2* initFunction = nullptr;

#if !MOBILEAPP

BackgroundSaveWatchdog::BackgroundSaveWatchdog(unsigned mobileAppDocId,
                                               ProcUtil::ThreadId savingTid)
    : _saveCompleted(false)
    , _watchdogThread(
          // mobileAppDocId is on the stack, so capture it by value.
          [mobileAppDocId, savingTid, this]()
          {
              ProcUtil::setThreadName("kitbgsv_" + Util::encodeId(mobileAppDocId, 3) + "_wdg");

              const auto timeout = std::chrono::seconds(
                  ConfigUtil::getInt("per_document.bgsave_timeout_secs", 120));

              const auto saveStart = std::chrono::steady_clock::now();

              std::unique_lock<std::mutex> lock(_watchdogMutex);

              LOG_TRC("Starting bgsave watchdog with " << timeout << " timeout");
              if (_watchdogCV.wait_for(lock, timeout,
                                       [this]() { return _saveCompleted.load(); }))
              {
                  // Done!
                  LOG_TRC("BgSave finished in time");
              }
              else
              {
                  auto saveDuration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - saveStart);

                  // Failed! Release the lock before the shutdown/kill sequence.
                  lock.unlock();

                  LOG_WRN("BgSave timed out and will self-destroy process " << getpid() <<
                          " (config timeout: " << timeout << ", real timeout: " << saveDuration << ")");
                  Log::shutdown(); // Flush logs.
                  // this attempts to get the saving-thread to generate a backtrace
                  ProcUtil::killThreadById(savingTid, SIGABRT);

                  // It is possible that this process will not exit cleanly after
                  // handling SIGABRT, so instead after some time fall-back to this:

                  // raise(3) will exit the current thread, not the process.
                  // coverity[sleep : SUPPRESS] - don't report sleep with lock held
                  sleep(30); // long enough for a trace ?
                  std::cerr << "BgSave failed to terminate after SIGABRT - will hard self-destroy process " << getpid() << std::endl;
                  ::kill(0, SIGKILL); // kill(2) is trapped by seccomp.
              }
          })
{
}

BackgroundSaveWatchdog::~BackgroundSaveWatchdog()
{
    if (!_saveCompleted)
    {
        LOG_WRN("BgSave watchdog for " << getpid()
                                       << " is destroyed while save hadn't yet completed");
        complete(); // Clean up.
    }
}

void BackgroundSaveWatchdog::complete()
{
    _saveCompleted = true;
    _watchdogCV.notify_all();
    if (_watchdogThread.joinable())
        _watchdogThread.join();
}

void Document::shutdownBackgroundWatchdog()
{
    if (BackgroundSaveWatchdog::Instance)
        BackgroundSaveWatchdog::Instance->complete();
}

#endif // !MOBILEAPP

namespace
{
    // for later consistency checking.
    static std::string UserDirPath;
    static std::string InstDirPath;

    std::string pathFromFileURL(const std::string &uri)
    {
        const std::string decoded = Uri::decode(uri);
        if (decoded.rfind("file://", 0) != 0)
        {
            LOG_ERR("Asked to load a very unusual file path: '" << uri << "' -> '" << decoded << "'");
            return std::string();
        }
        return decoded.substr(7);
    }

    [[maybe_unused]]
    void consistencyCheckFileExists(const std::string &uri)
    {
        std::string path = pathFromFileURL(uri);
        if (path.empty())
            return;
        FileUtil::Stat stat(path);
        if (!stat.good() && stat.isFile())
            LOG_ERR("Fatal system error: created file passed into document doesn't exist: '" << path << "'");
        else
            LOG_TRC("File path '" << path << "' exists of length " << stat.size());

        consistencyCheckJail();
    }

#if !defined(BUILDING_TESTS) && !MOBILEAPP
    enum class LinkOrCopyType: std::uint8_t
    {
        All,
        LO
    };
    LinkOrCopyType linkOrCopyType;
    std::string sourceForLinkOrCopy;
    Poco::Path destinationForLinkOrCopy;
    bool forceInitialCopy; // some stackable file-systems have very slow first hard link creation
    std::string linkableForLinkOrCopy; // Place to stash copies that we can hard-link from
    std::chrono::time_point<std::chrono::steady_clock> linkOrCopyStartTime;
    bool linkOrCopyVerboseLogging = false;
    unsigned linkOrCopyFileCount = 0; // Track to help quantify the link-or-copy performance.
    constexpr unsigned SlowLinkOrCopyLimitInSecs = 2; // After this many seconds, start spamming the logs.

    bool detectSlowStackingFileSystem([[maybe_unused]] const std::string& directory)
    {
#ifdef __linux__
#ifndef OVERLAYFS_SUPER_MAGIC
// From linux/magic.h.
#define OVERLAYFS_SUPER_MAGIC   0x794c7630
#endif
        struct statfs fs;
        if (::statfs(directory.c_str(), &fs) != 0)
        {
            LOG_SYS("statfs failed on '" << directory << "'");
            return false;
        }
        switch (fs.f_type) {
//        case FUSE_SUPER_MAGIC: ?
        case OVERLAYFS_SUPER_MAGIC:
            return true;
        default:
            return false;
        }
#else
        return false;
#endif
    }

    /// Returns the LinkOrCopyType as a human-readable string (for logging).
    std::string linkOrCopyTypeString(LinkOrCopyType type)
    {
        switch (type)
        {
            case LinkOrCopyType::LO:
                return "CollaboraOffice";
            case LinkOrCopyType::All:
                return "all";
            default:
                assert(!"Unknown LinkOrCopyType.");
                return "unknown";
        }
    }

    bool shouldCopyDir(const char *path)
    {
        switch (linkOrCopyType)
        {
        case LinkOrCopyType::LO:
            return path != std::string_view("program/wizards") &&
                   path != std::string_view("sdk") &&
                   path != std::string_view("debugsource") &&
                   path != std::string_view("share/basic") &&
                   !std::string_view(path).starts_with(std::string_view("share/extentions/dict")) &&
                   path != std::string_view("share/Scripts/java") &&
                   path != std::string_view("share/Scripts/javascript") &&
                   path != std::string_view("share/config/wizard") &&
                   path != std::string_view("readmes") &&
                   path != std::string_view("help");
        default: // LinkOrCopyType::All
            return true;
        }
    }

    bool shouldLinkFile(const char *path)
    {
        switch (linkOrCopyType)
        {
        case LinkOrCopyType::LO:
        {
            if (strstr(path, "LICENSE") || strstr(path, "EULA") || strstr(path, "CREDITS")
                || strstr(path, "NOTICE"))
                return false;

            const char* dot = strrchr(path, '.');
            if (!dot)
                return true;

            if (dot == std::string_view(".dbg"))
                return false;

            if (dot == std::string_view(".so"))
            {
                // NSS is problematic ...
                if (strstr(path, "libnspr4") || strstr(path, "libplds4") ||
                    strstr(path, "libplc4") || strstr(path, "libnss3") ||
                    strstr(path, "libnssckbi") || strstr(path, "libnsutil3") ||
                    strstr(path, "libssl3") || strstr(path, "libsoftokn3") ||
                    strstr(path, "libsqlite3") || strstr(path, "libfreeblpriv3"))
                    return true;

                // As is Python ...
                if (strstr(path, "python-core"))
                    return true;

                // otherwise drop the rest of the code.
                return false;
            }
            const char *vers;
            if ((vers = strstr(path, ".so."))) // .so.[digit]+
            {
                for(int i = sizeof (".so."); vers[i] != '\0'; ++i)
                    if (!isdigit(vers[i]) && vers[i] != '.')
                        return true;
                return false;
            }
            return true;
        }
        default: // LinkOrCopyType::All
            return true;
        }
    }

    void linkOrCopyFile(const char* fpath, const std::string& newPath)
    {
        ++linkOrCopyFileCount;
        if (linkOrCopyVerboseLogging)
            LOG_INF("Linking file \"" << fpath << "\" to \"" << newPath << '"');

        if (!forceInitialCopy)
        {
            // first try a simple hard-link
            if (link(fpath, newPath.c_str()) == 0)
                return;
        }
        // else always copy before linking to linkable/

        // incrementally build our 'linkable/' copy nearby
        static bool canChown = true; // only if we can get permissions right
        if ((forceInitialCopy || errno == EXDEV) && canChown)
        {
            // then copy somewhere closer and hard link from there
            if (!forceInitialCopy)
                LOG_TRC("link(\"" << fpath << "\", \"" << newPath << "\") failed: " << strerror(errno)
                        << ". Will try to link template.");

            std::string linkableCopy = linkableForLinkOrCopy + fpath;
            if (::link(linkableCopy.c_str(), newPath.c_str()) == 0)
                return;

            if (errno == ENOENT)
            {
                File(Path(linkableCopy).parent()).createDirectories();
                if (!FileUtil::copy(fpath, linkableCopy, /*log=*/false, /*throw_on_error=*/false))
                    LOG_TRC("Failed to create linkable copy [" << fpath << "] to [" << linkableCopy.c_str() << "]");
                else {
                    // Match system permissions, so a file we can write is not shared across jails.
                    struct stat ownerInfo;
                    if (::stat(fpath, &ownerInfo) != 0 ||
                        ::chown(linkableCopy.c_str(), ownerInfo.st_uid, ownerInfo.st_gid) != 0)
                    {
                        LOG_ERR("Failed to stat or chown " << ownerInfo.st_uid << ":" << ownerInfo.st_gid <<
                                " " << linkableCopy << ": " << strerror(errno) << " missing cap_chown?, disabling linkable");
                        unlink(linkableCopy.c_str());
                        canChown = false;
                    }
                    else if (::link(linkableCopy.c_str(), newPath.c_str()) == 0)
                        return;
                }
            }
            LOG_TRC("link(\"" << linkableCopy << "\", \"" << newPath << "\") failed: " << strerror(errno)
                    << ". Cannot create linkable copy.");
        }

        static bool warned = false;
        if (!warned)
        {
            LOG_ERR("link(\"" << fpath << "\", \"" << newPath.c_str() << "\") failed: " << strerror(errno)
                    << ". Very slow copying path triggered.");
            warned = true;
        } else
            LOG_TRC("link(\"" << fpath << "\", \"" << newPath.c_str() << "\") failed: " << strerror(errno)
                    << ". Will copy.");
        if (!FileUtil::copy(fpath, newPath, /*log=*/false, /*throw_on_error=*/false))
        {
            LOG_FTL("Failed to copy or link [" << fpath << "] to [" << newPath << "]. Exiting.");
            Util::forcedExit(EX_SOFTWARE);
        }
    }

    int linkOrCopyFunction(const char *fpath,
                           const struct stat* sb,
                           int typeflag,
                           struct FTW* /*ftwbuf*/)
    {
        if (fpath == sourceForLinkOrCopy)
        {
            LOG_TRC("nftw: Skipping redundant path: " << fpath);
            return FTW_CONTINUE;
        }

        if (!linkOrCopyVerboseLogging)
        {
            const auto durationInSecs = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - linkOrCopyStartTime);
            if (durationInSecs.count() > SlowLinkOrCopyLimitInSecs)
            {
                LOG_WRN("Linking/copying files from "
                        << sourceForLinkOrCopy << " to " << destinationForLinkOrCopy.toString()
                        << " is taking too much time. Enabling verbose link/copy logging.");
                linkOrCopyVerboseLogging = true;
            }
        }

        assert(fpath[sourceForLinkOrCopy.size()] == '/');
        const char* relativeOldPath = fpath + sourceForLinkOrCopy.size() + 1;
        const Poco::Path newPath(destinationForLinkOrCopy, Poco::Path(relativeOldPath));

        switch (typeflag)
        {
        case FTW_F:
        case FTW_SLN:
            Poco::File(newPath.parent()).createDirectories();

            if (shouldLinkFile(relativeOldPath))
                linkOrCopyFile(fpath, newPath.toString());
            break;
        case FTW_D:
            {
                struct stat st;
                if (stat(fpath, &st) == -1)
                {
                    LOG_SYS("nftw: stat(\"" << fpath << "\") failed");
                    return FTW_STOP;
                }
                if (!shouldCopyDir(relativeOldPath))
                {
                    LOG_TRC("nftw: Skipping redundant path: " << relativeOldPath);
                    return FTW_SKIP_SUBTREE;
                }

                Poco::File(newPath).createDirectories();
                struct utimbuf ut;
                ut.actime = st.st_atime;
                ut.modtime = st.st_mtime;
                if (utime(newPath.toString().c_str(), &ut) == -1)
                {
                    LOG_SYS("nftw: utime(\"" << newPath.toString() << "\") failed");
                    return FTW_STOP;
                }
            }
            break;
        case FTW_SL:
            {
                const std::size_t size = sb->st_size;
                std::vector<char> target(size + 1);
                char* target_data = target.data();
                const ssize_t written = readlink(fpath, target_data, size);
                if (written <= 0 || static_cast<std::size_t>(written) > size)
                {
                    LOG_SYS("nftw: readlink(\"" << fpath << "\") failed");
                    Util::forcedExit(EX_SOFTWARE);
                }
                target_data[written] = '\0';

                Poco::File(newPath.parent()).createDirectories();
                if (symlink(target_data, newPath.toString().c_str()) == -1)
                {
                    LOG_SYS("nftw: symlink(\"" << target_data << "\", \"" << newPath.toString()
                                               << "\") failed");
                    return FTW_STOP;
                }
            }
            break;
            case FTW_DNR:
                LOG_ERR("nftw: Cannot read directory '" << fpath << '\'');
                return FTW_STOP;
            case FTW_NS:
                LOG_ERR("nftw: stat failed for '" << fpath << '\'');
                return FTW_STOP;
            default:
                LOG_FTL("nftw: unexpected typeflag: '" << typeflag);
                assert(!"nftw: unexpected typeflag.");
                break;
        }

        return FTW_CONTINUE;
    }

    void linkOrCopy(const std::string& source, const Poco::Path& destination, const std::string& linkable,
                    LinkOrCopyType type)
    {
        std::string resolved = FileUtil::realpath(source);
        if (resolved != source)
        {
            LOG_DBG("linkOrCopy: Using real path [" << resolved << "] instead of original link ["
                                                    << source << "].");
        }

        LOG_INF("linkOrCopy " << linkOrCopyTypeString(type) << " from [" << resolved << "] to ["
                              << destination.toString() << "].");

        linkOrCopyType = type;
        sourceForLinkOrCopy = resolved;
        if (sourceForLinkOrCopy.back() == '/')
            sourceForLinkOrCopy.pop_back();
        destinationForLinkOrCopy = destination;
        linkableForLinkOrCopy = linkable;
        linkOrCopyFileCount = 0;
        linkOrCopyStartTime = std::chrono::steady_clock::now();
        forceInitialCopy = detectSlowStackingFileSystem(destination.toString());

        if (nftw(resolved.c_str(), linkOrCopyFunction, 10, FTW_ACTIONRETVAL|FTW_PHYS) == -1)
        {
            LOG_ERR("linkOrCopy: nftw() failed for '" << resolved << '\'');
        }

        if (linkOrCopyVerboseLogging)
        {
            linkOrCopyVerboseLogging = false;
            const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - linkOrCopyStartTime).count();
            const double seconds = (ms + 1) / 1000.; // At least 1ms to avoid div-by-zero.
            const auto rate = linkOrCopyFileCount / seconds;
            LOG_INF("Linking/Copying of " << linkOrCopyFileCount << " files from " << resolved
                                          << " to " << destinationForLinkOrCopy.toString()
                                          << " finished in " << seconds << " seconds, or " << rate
                                          << " files / second.");
        }
    }

#if CODE_COVERAGE
    std::string childRootForGCDAFiles;
    std::string sourceForGCDAFiles;
    std::string destForGCDAFiles;

    int linkGCDAFilesFunction(const char* fpath, const struct stat*, int typeflag,
                              struct FTW* /*ftwbuf*/)
    {
        const std::string path = fpath;
        if (path == sourceForGCDAFiles)
        {
            LOG_TRC("nftw: Skipping redundant path: " << fpath);
            return FTW_CONTINUE;
        }

        if (path.starts_with(childRootForGCDAFiles))
        {
            LOG_TRC("nftw: Skipping childRoot subtree: " << fpath);
            return FTW_SKIP_SUBTREE;
        }

        assert(path.size() >= sourceForGCDAFiles.size());
        assert(fpath[sourceForGCDAFiles.size()] == '/');
        const char* relativeOldPath = fpath + sourceForGCDAFiles.size() + 1;
        const Poco::Path newPath(destForGCDAFiles, Poco::Path(relativeOldPath));

        switch (typeflag)
        {
            case FTW_F:
            case FTW_SLN:
            {
                const char* dot = strrchr(relativeOldPath, '.');
                if (dot && dot == std::string_view(".gcda"))
                {
                    Poco::File(newPath.parent()).createDirectories();
                    if (link(fpath, newPath.toString().c_str()) != 0)
                    {
                        LOG_SYS("nftw: Failed to link [" << fpath << "] -> [" << newPath.toString()
                                                         << ']');
                    }
                }
            }
            break;
            case FTW_D:
            case FTW_SL:
                break;
            case FTW_DNR:
                LOG_ERR("nftw: Cannot read directory '" << fpath << '\'');
                break;
            case FTW_NS:
                LOG_ERR("nftw: stat failed for '" << fpath << '\'');
                break;
            default:
                LOG_FTL("nftw: unexpected typeflag: '" << typeflag);
                assert(!"nftw: unexpected typeflag.");
                break;
        }

        return FTW_CONTINUE;
    }

    /// Link .gcda (gcov) files from the src directory into the jail.
    /// We need this so we can easily extract the profile data from within
    /// the jail. Otherwise, we lose coverage info of the kit process.
    void linkGCDAFiles(const std::string& destPath)
    {
        Poco::Path sourcePathInJail(destPath);
        const auto sourcePath = std::string(DEBUG_ABSSRCDIR);
        sourcePathInJail.append(sourcePath);
        Poco::File(sourcePathInJail).createDirectories();
        LOG_INF("Linking .gcda files from " << sourcePath << " -> " << sourcePathInJail.toString());

        const auto childRootPtr = std::getenv("BASE_CHILD_ROOT");
        if (childRootPtr == nullptr || strlen(childRootPtr) == 0)
        {
            LOG_ERR("Cannot collect code-coverage stats for the Kit processes. BASE_CHILD_ROOT "
                    "envar missing.");
            return;
        }

        // Trim the trailing /.
        const std::string childRoot = childRootPtr;
        const size_t last = childRoot.find_last_not_of('/');
        if (last != std::string::npos)
            childRootForGCDAFiles = childRoot.substr(0, last + 1);
        else
            childRootForGCDAFiles = childRoot;

        sourceForGCDAFiles = sourcePath;
        destForGCDAFiles = sourcePathInJail.toString() + '/';
        LOG_INF("nftw .gcda files from " << sourceForGCDAFiles << " -> " << destForGCDAFiles << " ("
                                         << childRootForGCDAFiles << ')');

        if (nftw(sourcePath.c_str(), linkGCDAFilesFunction, 10, FTW_ACTIONRETVAL | FTW_PHYS) == -1)
        {
            LOG_ERR("linkGCDAFiles: nftw() failed for '" << sourcePath << '\'');
        }
    }
#endif

#if HAVE_LIBCAP
    void dropCapability(cap_value_t capability)
    {
        cap_t caps;
        cap_value_t cap_list[] = { capability };

        caps = cap_get_proc();
        if (caps == nullptr)
        {
            LOG_SFL("cap_get_proc() failed");
            Util::forcedExit(EX_SOFTWARE);
        }

        char *capText = cap_to_text(caps, nullptr);
        LOG_TRC("Capabilities first: " << capText);
        cap_free(capText);

        if (cap_set_flag(caps, CAP_EFFECTIVE, N_ELEMENTS(cap_list), cap_list, CAP_CLEAR) == -1 ||
            cap_set_flag(caps, CAP_PERMITTED, N_ELEMENTS(cap_list), cap_list, CAP_CLEAR) == -1)
        {
            LOG_SFL("cap_set_flag() failed");
            Util::forcedExit(EX_SOFTWARE);
        }

        if (cap_set_proc(caps) == -1)
        {
            LOG_SFL("cap_set_proc() failed");
            Util::forcedExit(EX_SOFTWARE);
        }

        capText = cap_to_text(caps, nullptr);
        LOG_TRC("Capabilities now: " << capText);
        cap_free(capText);

        cap_free(caps);
    }
#endif // __FreeBSD__
#endif // BUILDING_TESTS
} // namespace

Document::Document(const std::shared_ptr<kit::Office>& loKit, const std::string& jailId,
                   const std::string& docKey, const std::string& docId, const std::string& url,
                   const std::shared_ptr<WebSocketHandler>& websocketHandler,
                   unsigned mobileAppDocId)
    : _loKit(loKit)
    , _jailId(jailId)
    , _docKey(docKey)
    , _docId(docId)
    , _url(url)
    , _obfuscatedFileId(Uri::getFilenameFromURL(Uri::decode(docKey)))
    , _queue(new KitQueue(*this))
    , _websocketHandler(websocketHandler)
    , _modified(ModifiedState::UnModified)
    , _isBgSaveProcess(false)
    , _isBgSaveDisabled(false)
    , _trimIfInactivePostponed(false)
    , _haveDocPassword(false)
    , _isDocPasswordProtected(false)
    , _docPasswordType(DocumentPasswordType::ToView)
    , _stop(false)
    , _deltaGen(new DeltaGenerator())
    , _editorId(-1)
    , _editorChangeWarning(false)
    , _lastMemTrimTime(std::chrono::steady_clock::now())
    , _mobileAppDocId(mobileAppDocId)
    , _duringLoad(0)
    , _bgSavesOngoing(0)
{
    LOG_INF("Document ctor for [" << _docKey <<
            "] url [" << anonymizeUrl(_url) << "] on child [" << _jailId <<
            "] and id [" << _docId << "].");
    assert(_loKit);
#if !MOBILEAPP
    assert(singletonDocument == nullptr);
    singletonDocument = this;
#endif
    // Open file for UI Logging
    if (Log::isLogUIEnabled())
    {
        logUiCmd.createTmpFile(_docId);
    }
}

Document::~Document()
{
    LOG_INF("~Document dtor for [" << _docKey <<
            "] url [" << anonymizeUrl(_url) << "] on child [" << _jailId <<
            "] and id [" << _docId << "]. There are " <<
            _sessions.size() << " views.");

    // Wait for the callback worker to finish.
    _stop = true;

    for (const auto& session : _sessions)
    {
        session.second->resetDocManager();
    }

#if defined(IOS) || defined(MACOS) || defined(_WIN32) || defined(QTAPP)
    DocumentData::deallocate(_mobileAppDocId);
#endif

}

/// Post the message - in the unipoll world we're in the right thread anyway
bool Document::postMessage(const std::string_view data, const WSOpCode code) const
{
    if (_isBgSaveProcess)
    {
        auto socket = _saveProcessParent.lock();
        if (socket)
        {
            LOG_TRC("postMessage forwarding to parent of save process: "
                    << getAbbreviatedMessage(data));
            if (code != WSOpCode::Text)
            {
                LOG_WRN("save process unexpectedly sending binary message to parent: "
                        << getAbbreviatedMessage(data));
                assert(false);
                return false;
            }

            return socket->sendMessage(data.data(), data.size(), code, /*flush=*/true) > 0;
        }

        LOG_TRC("Failed to forward to parent of save process: connection closed");
        return false;
    }

    if (!_websocketHandler)
    {
        LOG_ERR("Child Doc: Bad socket while sending: " << getAbbreviatedMessage(data));
        return false;
    }

    LOG_TRC("postMessage called with: " << getAbbreviatedMessage(data));
    _websocketHandler->sendMessage(data.data(), data.size(), code, /*flush=*/true);
    return true;
}

bool Document::createSession(const std::string& sessionId)
{
#if defined(BUILDING_TESTS)
    LOG_ERR("createSession stubbed for tests for " << sessionId);
    return false;
#else
    try
    {
        if (_sessions.find(sessionId) != _sessions.end())
        {
            LOG_ERR("Session [" << sessionId << "] on url [" << anonymizeUrl(_url) << "] already exists.");
            return true;
        }

        LOG_INF("Creating " << (_sessions.empty() ? "first" : "new") <<
                " session for url: " << anonymizeUrl(_url) << " for sessionId: " <<
                sessionId << " on jailId: " << _jailId);

        auto session = std::make_shared<ChildSession>(
            _websocketHandler, sessionId,
            _jailId, JailRoot, *this);
        if (!Util::isMobileApp())
            UnitKit::get().postKitSessionCreated(session.get());
        _sessions.emplace(sessionId, session);
        _deltaGen->setSessionCount(_sessions.size());

        const int viewId = session->getViewId();
        _lastUpdatedAt[viewId] = std::chrono::steady_clock::now();
        _speedCount[viewId] = 0;

        LOG_INF("New session [" << sessionId << "] created. Have " << _sessions.size()
                                << " sessions now");

        updateActivityHeader();
        return true;
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Exception while creating session [" << sessionId <<
                "] on url [" << anonymizeUrl(_url) << "] - '" << ex.what() << "'.");
        return false;
    }
#endif
}

std::size_t Document::purgeSessions()
{
    std::vector<std::shared_ptr<ChildSession>> deadSessions;
    std::size_t num_sessions = 0;
    {
        // If there are no live sessions, we don't need to do anything at all and can just
        // bluntly exit, no need to clean up our own data structures. Also, there is a bug that
        // causes the deadSessions.clear() call below to crash in some situations when the last
        // session is being removed.
        for (auto it = _sessions.cbegin(); it != _sessions.cend(); )
        {
            if (it->second->isCloseFrame())
            {
                LOG_DBG("Removing session [" << it->second->getId() << ']');
                deadSessions.push_back(it->second);
                it = _sessions.erase(it);
            }
            else
            {
                ++it;
            }
        }

        num_sessions = _sessions.size();
#if !MOBILEAPP
        if (num_sessions == 0)
        {
            LOG_FTL("Document [" << anonymizeUrl(_url) << "] has no more views, exiting bluntly.");
            flushAndExit(EX_OK);
        }
#endif
    }

    if (deadSessions.size() > 0 )
        LOG_TRC("Purging " << deadSessions.size() <<
                " dead sessions, with " << num_sessions <<
                " active sessions.");

    // Don't destroy sessions while holding our lock.
    // We may deadlock if a session is waiting on us
    // during callback initiated while handling a command
    // and the dtor tries to take its lock (which is taken).
    deadSessions.clear();

    return num_sessions;
}

/// Set Document password for given URL
void Document::setDocumentPassword(int passwordType)
{
    // Log whether the document is password protected and a password is provided
    LOG_INF("setDocumentPassword: passwordProtected=" << _isDocPasswordProtected <<
            " passwordProvided=" << _haveDocPassword);

    if (_isDocPasswordProtected && _haveDocPassword)
    {
        // it means this is the second attempt with the wrong password; abort the load operation
        _loKit->setDocumentPassword(_jailedUrl.c_str(), nullptr);
        return;
    }

    // One thing for sure, this is a password protected document
    _isDocPasswordProtected = true;
    if (passwordType == KIT_CALLBACK_DOCUMENT_PASSWORD)
        _docPasswordType = DocumentPasswordType::ToView;
    else if (passwordType == KIT_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY)
        _docPasswordType = DocumentPasswordType::ToModify;

    LOG_INF("Calling _loKit->setDocumentPassword");
    if (_haveDocPassword)
        _loKit->setDocumentPassword(_jailedUrl.c_str(), _docPassword.c_str());
    else
        _loKit->setDocumentPassword(_jailedUrl.c_str(), nullptr);
    LOG_INF("setDocumentPassword returned.");
}

void Document::renderTiles(TileCombined &tileCombined)
{
    // Find a session matching our view / render settings.
    const auto session = _sessions.findByCanonicalId(tileCombined.getCanonicalViewId());
    if (!session)
    {
        LOG_ERR("Session is not found. Maybe exited after rendering request.");
        return;
    }

    if (!_loKitDocument)
    {
        LOG_ERR("Tile rendering requested before loading document.");
        return;
    }

    if (_loKitDocument->getViewsCount() <= 0)
    {
        LOG_ERR("Tile rendering requested without views.");
        return;
    }

    // if necessary select a suitable rendering view eg. with 'show non-printing chars'
    if (tileCombined.getCanonicalViewId() != CanonicalViewId::None)
        _loKitDocument->setView(session->getViewId());

    const auto blenderFunc = [&](unsigned char* data, int offsetX, int offsetY,
                                 std::size_t pixmapWidth, std::size_t pixmapHeight,
                                 int pixelWidth, int pixelHeight, COKitTileMode mode) {
        if (session->watermark())
            session->watermark()->blending(data, offsetX, offsetY, pixmapWidth, pixmapHeight,
                                           pixelWidth, pixelHeight, mode);
    };

    const auto postMessageFunc = [&](const char* buffer, std::size_t length)
    { postMessage(std::string_view(buffer, length), WSOpCode::Binary); };

    if (!RenderTiles::doRender(_loKitDocument, *_deltaGen, tileCombined, _deltaPool,
                               blenderFunc, postMessageFunc, _mobileAppDocId,
                               session->getCanonicalViewId(), session->getDumpTiles()))
    {
        LOG_DBG("All tiles skipped, not producing empty tilecombine: message");
        return;
    }
}

bool Document::sendFrame(const std::string_view data, WSOpCode opCode) const
{
    try
    {
        return postMessage(data, opCode);
    }
    catch (const Exception& exc)
    {
        LOG_ERR("Document::sendFrame: Exception: " << exc.displayText() <<
                (exc.nested() ? "( " + exc.nested()->displayText() + ')' : ""));
    }

    return false;
}

void Document::bgSaveEnded()
{
    _bgSavesOngoing--;
    if (!_bgSavesOngoing)
    {
        // Delay the next trimAfterInactivity check to let our state
        // settle before trimming.
        _lastMemTrimTime = std::chrono::steady_clock::now();
    }
}

void Document::trimIfInactive()
{
    // Don't perturb memory un-necessarily
    if (_isBgSaveProcess)
        return;
    if (_bgSavesOngoing)
    {
        // Postpone until trimAfterInactivity after bgsave has completed.
        _trimIfInactivePostponed = true;
        return;
    }

    _trimIfInactivePostponed = false;

    // FIXME: multi-document mobile optimization ?
    for (const auto& it : _sessions)
    {
        if (it.second->isActive())
        {
            LOG_TRC("have active session, don't trim");
            return;
        }
    }
    // TODO: be more clever - detect if we mutated the document
    // recently, measure memory pressure etc.
    LOG_DBG("Sessions are all inactive - trim memory");
    SigUtil::addActivity("trimIfInactive");
    _loKit->trimMemory(4096);
    _deltaGen->dropCache();
    // Inform docbroker that document has (deep) trimmed memory
    sendTextFrame("memorytrimmed:");
}

void Document::trimAfterInactivity()
{
    // Don't perturb memory un-necessarily
    if (_isBgSaveProcess || _bgSavesOngoing)
        return;

    if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() -
                                                         _lastMemTrimTime) < std::chrono::seconds(30))
    {
        return;
    }

    // If a deep trim was missed due to an ongoing bg save then enable that to happen now.
    if (_trimIfInactivePostponed)
    {
        trimIfInactive();
        return;
    }

    LOG_TRC("Should we trim our caches ?");
    double minInactivityMs = std::numeric_limits<double>::max();
    for (const auto& it : _sessions)
    {
        minInactivityMs = std::min(it.second->getInactivityMS(), minInactivityMs);
    }

    if (minInactivityMs >= 9999)
    {
        LOG_DBG("Trimming Core caches");
        SigUtil::addActivity("trimAfterInactivity");
        _loKit->trimMemory(1024);

        _lastMemTrimTime = std::chrono::steady_clock::now();
    }
}

/* static */ void Document::GlobalCallback(const int type, const char* p, void* data)
{
    if (SigUtil::getTerminationFlag())
        return;

    // unusual COKit event from another thread,
    // data - is Document with process' lifetime.
    if (pushToMainThread(GlobalCallback, type, p, data))
        return;

    const std::string payload = p ? p : "(nil)";
    Document* self = static_cast<Document*>(data);

    if (type == KIT_CALLBACK_PROFILE_FRAME)
    {
        // We must send the trace data to the WSD process for output

        LOG_TRC("Document::GlobalCallback " << kitCallbackTypeToString(type) << ": " << payload.length() << " bytes.");

        self->sendTextFrame("traceevent: \n" + payload);
        return;
    }

    LOG_TRC("Document::GlobalCallback " << kitCallbackTypeToString(type) <<
            " [" << payload << "].");

    if (type == KIT_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY ||
        type == KIT_CALLBACK_DOCUMENT_PASSWORD)
    {
        // Mark the document password type.
        self->setDocumentPassword(type);
        return;
    }
    else if (type == KIT_CALLBACK_STATUS_INDICATOR_START ||
             type == KIT_CALLBACK_STATUS_INDICATOR_SET_VALUE ||
             type == KIT_CALLBACK_STATUS_INDICATOR_FINISH)
    {
        for (auto& it : self->_sessions)
        {
            const std::shared_ptr<ChildSession>& session = it.second;
            if (!session->isCloseFrame())
                session->loKitCallback(type, payload);
        }
        return;
    }
    else if (type == KIT_CALLBACK_JSDIALOG || type == KIT_CALLBACK_HYPERLINK_CLICKED)
    {
        if (self->_sessions.size() == 1)
        {
            auto it = self->_sessions.begin();
            const std::shared_ptr<ChildSession>& session = it->second;
            if (session && !session->isCloseFrame())
            {
                session->loKitCallback(type, payload);
                if (self->isLoadOngoing() && !self->processInputEnabled())
                {
                    LOG_DBG("Enable processing input due to event of " << type << " during load");
                    session->getProtocol()->enableProcessInput(true);
                }
                return;
            }
        }
    }

    // Broadcast leftover status indicator callbacks to all clients
    self->broadcastCallbackToClients(type, payload);
}

/* static */ void Document::ViewCallback(const int type, const char* p, void* data)
{
    if (SigUtil::getTerminationFlag())
        return;

    // unusual COKit event from another thread.
    // data - is CallbackDescriptors which share process' lifetime.
    if (pushToMainThread(ViewCallback, type, p, data))
        return;

    CallbackDescriptor* descriptor = static_cast<CallbackDescriptor*>(data);
    assert(descriptor && "Null callback data.");
    assert(descriptor->getDoc() && "Null Document instance.");

    std::unique_ptr<KitQueue> &queue = descriptor->getDoc()->_queue;
    assert(queue && "Null KitQueue.");

    const std::string payload = p ? p : "(nil)";
    LOG_TRC("Document::ViewCallback [" << descriptor->getViewId() <<
            "] [" << kitCallbackTypeToString(type) <<
            "] [" << payload << "].");

    if (type == KIT_CALLBACK_DOCUMENT_PASSWORD_RESET)
    {
        Document* document = descriptor->getDoc();
        Poco::JSON::Object::Ptr object;
        if (document && JsonUtil::parseJSON(payload, object))
        {
            std::string password = JsonUtil::getJSONValue<std::string>(object, "password");
            bool isToModify = JsonUtil::getJSONValue<bool>(object, "isToModify");
            document->_isDocPasswordProtected = !password.empty();
            document->_haveDocPassword = document->_isDocPasswordProtected;
            document->_docPassword = std::move(password);
            document->_docPasswordType =
                isToModify ? DocumentPasswordType::ToModify : DocumentPasswordType::ToView;
        }
        return;
    }
    else if (type == KIT_CALLBACK_VIEW_RENDER_STATE)
    {
        Document* document = descriptor->getDoc();
        if (document)
        {
            std::shared_ptr<ChildSession> session = document->findSessionByViewId(descriptor->getViewId());
            if (session)
            {
                if (!payload.empty())
                {
                    session->setViewRenderState(payload);
                    document->invalidateCanonicalId(session->getId());
                }
            }
            else
            {
                LOG_ERR("Cannot find session for viewId: " << descriptor->getViewId());
            }
        }
        else
        {
            // This shouldn't happen, but for consistency.
            LOG_ERR("Failed to downcast DocumentManagerInterface to Document");
        }
        return;
    }

    // merge various callback types together if possible
    if (type == KIT_CALLBACK_INVALIDATE_TILES)
    {
        // all views have to be in sync; FIXME: calc an issue here ?
        queue->putCallback(-1, type, payload);
    }
    else
        queue->putCallback(descriptor->getViewId(), type, payload);

    LOG_TRC("Document::ViewCallback end.");
}

/// Load a document (or view) and register callbacks.
bool Document::onLoad(const std::string& sessionId,
                      const std::string& uriAnonym,
                      const std::string& renderOpts)
{
    LOG_INF("Loading url [" << uriAnonym << "] for session [" << sessionId <<
            "] which has " << (_sessions.size() - 1) << " sessions.");

    Util::ReferenceHolder duringLoad(_duringLoad);

    // This shouldn't happen, but for sanity.
    const auto it = _sessions.find(sessionId);
    if (it == _sessions.end() || !it->second)
    {
        LOG_ERR("Cannot find session [" << sessionId << "] to load view for.");
        return false;
    }

    std::shared_ptr<ChildSession> session = it->second;
    try
    {
        if (load(session, renderOpts))
        {
            return true;
        }
    }
    catch (const std::exception &exc)
    {
        LOG_ERR("Exception while loading url [" << uriAnonym << "] for session [" << sessionId
                                                << "]: " << exc.what());
    }

    return false;
}

void Document::onUnload(const ChildSession& session)
{
    // This is called when we receive 'child-??? disconnect'.
    // First, we _sessions.erase(), which destroys the ChildSession instance.
    // We are called from ~ChildSession.

    const auto& sessionId = session.getId();
    LOG_INF("Unloading session [" << sessionId << "] on url [" << anonymizeUrl(_url) << ']');

    if (_loKitDocument == nullptr)
    {
        LOG_ERR("Unloading session [" << sessionId << "] without loKitDocument, exiting bluntly");
        flushAndExit(EX_OK);
        return;
    }

    // If we have no more sessions, we have nothing more to do.
    if (!Util::isMobileApp() && !haveLoadedSessions())
    {
        // Sanity check.
        std::ostringstream msg;
        const int views = _loKitDocument->getViewsCount();
        if (views > 1 || isBackgroundSaveProcess())
        {
            // Normally, this is a race between the save notification
            // sent from the background-save process being processed
            // by DocBroker and we processing the disconnection from
            // said background-save process before getting here.
            // However, this could also be an indication of a save
            // still in progress and DocBroker unloading--a bug.
            msg << " but " << views << " views"
                << (isBackgroundSaveProcess() ? " and background-save in progress" : "");
        }

        LOG_INF("Document [" << anonymizeUrl(_url) << "] has no more sessions" << msg.str()
                             << "; exiting bluntly");

        // Save UI log from kit to a permanent place
        if (Log::isLogUIEnabled())
        {
            logUiCmd.saveLogFile();
        }

        flushAndExit(EX_OK);
        return;
    }

    const int viewId = session.getViewId();

    // Unload the view.
    _loKitDocument->setView(viewId);
    _loKitDocument->registerCallback(nullptr, nullptr);
    _loKit->registerCallback(nullptr, nullptr);
    _loKitDocument->destroyView(viewId);

    // Since callback messages are processed on idle-timer,
    // we could receive callbacks after destroying a view.
    // Retain the CallbackDescriptor object, which is shared with Core.
    // Do not: _viewIdToCallbackDescr.erase(viewId);

    const int viewCount = _loKitDocument->getViewsCount();
    LOG_INF("Document [" << anonymizeUrl(_url) << "] session [" << sessionId << "] unloaded view ["
                         << viewId << "]. Have " << viewCount << " view"
                         << (viewCount != 1 ? "s" : "") << " and " << _sessions.size() << " session"
                         << (_sessions.size() != 1 ? "s" : ""));

    if (viewCount > 0)
    {
        // Broadcast updated view info
        notifyViewInfo();
    }
}

bool Document::haveLoadedSessions() const
{
    for (const auto& session : _sessions)
    {
        if (session.second->isDocLoaded())
        {
            return true;
        }
    }

    return false;
}

void Document::updateActivityHeader() const
{
    // pre-prepare and set details in case of a signal later
    std::stringstream ss;
    ss << "Session count: " << _sessions.size() << "\n";
    for (const auto& it : _sessions)
        ss << "\t" << it.second->getActivityState() << "\n";
    ss << "Commands:\n";
    SigUtil::setActivityHeader(ss.str());
}

bool Document::joinThreads()
{
    if (!getLOKit()->joinThreads())
        return false;

#if !MOBILEAPP
    if (SocketPoll::PollWatchdog)
        SocketPoll::PollWatchdog->joinThread();
#endif
    _deltaPool.stop();
    return true;
}

// Most threads are opportunisticaly created but some need to be started
void Document::startThreads()
{
    _deltaPool.start();

    getLOKit()->startThreads();

#if !MOBILEAPP
    if (SocketPoll::PollWatchdog)
        SocketPoll::PollWatchdog->startThread();
#endif
}

void Document::handleSaveMessage(const std::string &)
{
    LOG_TRC("Check save message");

#if !MOBILEAPP
    // if a bgsave process - now we can clean up.
    if (_isBgSaveProcess)
    {
        LOG_TRC("BgSave completed");

        // unregister the view callbacks
        const int viewCount = getLOKitDocument()->getViewsCount();
        std::vector<int> viewIds(viewCount);
        getLOKitDocument()->getViewIds(viewIds.data(), viewCount);
        for (const auto viewId : viewIds)
        {
            _loKitDocument->setView(viewId);
            _loKitDocument->registerCallback(nullptr, nullptr);
        }

        // cleanup any lingering file-system pieces
        _loKitDocument.reset();

        // any further messages are not interesting.
        if (_queue)
            _queue->clear();

        auto socket = _saveProcessParent.lock();
        if (socket)
        {
            LOG_TRC("Shutting down bgsv child's socket to parent kit post save");

            // We don't want to wait around for the parent's websocket
            socket->shutdownAfterWriting();

            // This means we don't get to send statechanged: .uno:ModifiedStatus
            // which is fine - we want to leave that to the Kit process.
        }
        else
            LOG_TRC("Shutting down already shutdown bgsv child's socket to parent kit post save");

        // Next step in the chain is BgSaveChildWebSocketHandler::onDisconnect
    }
#endif
}

#if !MOBILEAPP

// need to hold a reference on session in case it exits during async save
bool Document::forkToSave(const std::function<void()>& childSave, int viewId)
{
    if constexpr (Util::isMobileApp())
        return false;

    if (_isBgSaveProcess)
    {
        LOG_ERR("Serious error bgsv process trying to fork again");
        assert(false);
        return false;
    }

    if (_isBgSaveDisabled)
    {
        LOG_TRC("Skipping background save for bg save disabled process");
        return false;
    }

    ThreadDropper threadGuard;
    if (!threadGuard.dropThreads(this))
    {
        LOG_WRN("Failed to join threads before async save");
        return false;
    }

    size_t threads = getCurrentThreadCount();
    if (threads != 1)
    {
        LOG_DBG("Failed to ensure we have just one thread on 1st try, we have: " << threads);
        // Potentially the kernel can take time to cleanup after pthread_join
        int countDown = 10;
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if ((threads = getCurrentThreadCount()) == 1)
                break;
            countDown--;
            if (countDown < 1)
            {
                LOG_WRN("Failed to ensure we have just one thread for bgsave, "
                        "we have: " << threads << " synchronously saving");
                return false;
            }
        }
    }

    // Oddly we have seen this broken in the past.
    assert(processInputEnabled());

#if 0
    // TODO: compare FD count in a normal process with how
    // many we see open now.
    int expectFds = 2 // SocketPoll wakeups
        + 1; // socket to coolwsd
    int actualFds = fdCounter->count();
    if (actualFds != expectFds)
    {
        LOG_WRN("Can't background save: " << actualFds << " fds open; expect " << expectFds);
        return false;
    }
#endif

    const auto start = std::chrono::steady_clock::now();

    // TODO: close URPtoLoFDs and URPfromLoFDs and test
    if (isURPEnabled())
    {
        LOG_WRN("Can't background save with URP enabled");
        return false;
    }

    // FIXME: only do one of these at a time ...
    // FIXME: defer and queue a 2nd save if queued during save ...

    std::shared_ptr<StreamSocket> parentSocket, childSocket;
    if (!StreamSocket::socketpair(start, parentSocket, childSocket))
        return false;

    // To encode into the child process id for debugging
    static size_t numSaves = 0;
    numSaves++;

    Log::preFork();

    const pid_t pid = fork();

    if (!pid) // Child
    {
        Log::postFork();

        // sort out thread local variables to get logging right from
        // as early as possible.
        ProcUtil::setThreadName("kitbgsv_" + Util::encodeId(_mobileAppDocId, 3) + '_' +
                            Util::encodeId(numSaves, 3));
        _isBgSaveProcess = true;

        SigUtil::addActivity("forked background save process: " +
                             std::to_string(getpid()));

        threadGuard.clear();

        SigUtil::dieOnParentDeath();

        childSocket.reset();
        // now we just have a single socket to our parent

        Util::sleepFromEnvIfSet("KitBackgroundSave", "SLEEPBACKGROUNDFORDEBUGGER");

        assert(!BackgroundSaveWatchdog::Instance && "Unexpected to have BackgroundSaveWatchdog instance");
        BackgroundSaveWatchdog::Instance =
            std::make_unique<BackgroundSaveWatchdog>(_mobileAppDocId, ProcUtil::getThreadId());

        UnitKit::get().postBackgroundSaveFork();

        // Background save should run at a lower priority
#if 0
        // Disable changing priority for now
        int prio = ConfigUtil::getInt("per_document.bgsave_priority", 5);
        ProcUtil::setProcessAndThreadPriorities(getpid(), prio);
#endif

        // other queued messages should be handled in the parent kit
        if (_queue)
            _queue->clear();

        // Hard drop our previous connections to coolwsd and shared wakeups.
        KitSocketPoll::cleanupChildProcess();

        // close duplicate kit->wsd socket
        auto kitWs = std::static_pointer_cast<KitWebSocketHandler>(_websocketHandler);
        kitWs->shutdownForBackgroundSave();

        // now send messages to the parent instead of the kit.
        auto parentWs = std::make_shared<BgSaveChildWebSocketHandler>("bgsv_child_ws");
        parentSocket->setHandler(parentWs);
        parentSocket->setWebSocket(); // avoid http upgrade.
        _saveProcessParent = parentWs;

        // hand parentSocket to the main poll
        KitSocketPoll::getMainPoll()->insertNewSocket(parentSocket);
        parentWs.reset();

        getLOKit()->setForkedChild(true);

        const auto now = std::chrono::steady_clock::now();
        LOG_TRC("Background save process " << getpid() << " fork took " <<
                std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() << "ms");

        childSave();

        SigUtil::addActivity("background save process shutdown");

        // Wait now for an async save result from the core,
        // and head to handleSaveMessage
    }
    else // Still us
    {
        LOG_TRC("Spawned process " << pid << " to do background save");

        parentSocket.reset();
        // now we have a socket to the child: childSocket

        forceDocUnmodifiedForBgSave(viewId);

        auto bgSaveChild = std::make_shared<BgSaveParentWebSocketHandler>(
            "bgsv_kit_ws", pid, shared_from_this(),
            findSessionByViewId(viewId));
        childSocket->setHandler(bgSaveChild);
        childSocket->setWebSocket(); // avoid http upgrade.
        KitSocketPoll::getMainPoll()->insertNewSocket(childSocket);

        getLOKit()->setForkedChild(false);

        // now, rather than waiting for the destructor
        threadGuard.startThreads();

        // What better time than to reap while saving?
        reapZombieChildren();
    }
    return true;
}

void Document::reapZombieChildren()
{
    /// Normally, we reap children when the WebSocket is disconnected.
    /// See BgSaveParentWebSocketHandler::onDisconnect().
    /// This works well, except when the kernel is slower to unmap the
    /// pages, close descriptors, etc. than we do the waitpid(2) in
    /// onDisconnect(). It seems for small documents, that have a small
    /// memory footprint, unloading the process is fast, and we reap it.
    /// For large documents, however, the process ends up a zombie.
    /// Here, we reap any zombies that might exist--at most 1.
    for (;;)
    {
        const auto [ret, sig] = SigUtil::reapZombieChild(-1, /*sighandler=*/false);
        if (ret <= 0)
        {
            break;
        }
    }
}

#endif // !MOBILEAPP

namespace
{
// No need to actually send the values of some keys to the client, it's enough to know if these are
// provided or not. Replace the actual content with a placeholder.
void replaceKeysWithPlaceholder(std::string& json, std::initializer_list<std::string>& keys)
{
    try
    {
        if (!json.empty())
        {
            Parser parser;
            Poco::Dynamic::Var var = parser.parse(json);
            Object::Ptr jsonObj = var.extract<Object::Ptr>();
            for (const auto& key : keys)
            {
                std::string value;
                JsonUtil::findJSONValue(jsonObj, key, value);
                if (!value.empty())
                {
                    jsonObj->set(key, " ");
                }
            }
            std::ostringstream jsonStream;
            jsonObj->stringify(jsonStream);
            json = jsonStream.str();
        }
    }
    catch(const Poco::BadCastException& exception)
    {
        LOG_DBG("user private data is not a dictionary: " << exception.what());
    }
}
}

void Document::notifyViewInfo()
{
    // Get the list of view ids from the core
    const int viewCount = getLOKitDocument()->getViewsCount();
    std::vector<int> viewIds(viewCount);
    getLOKitDocument()->getViewIds(viewIds.data(), viewCount);

    const std::map<int, UserInfo> viewInfoMap = getViewInfo();

    const std::map<std::string, int> viewColorsMap = getViewColors();

    // Double check if list of viewids from core and our list matches,
    // and create an array of JSON objects containing id and username

    std::map<int, std::string> viewStrings; // viewId -> public data string

    for (const auto& viewId : viewIds)
    {
        std::ostringstream oss;
        oss << "\"id\":" << viewId << ',';
        int color = 0;
        const auto itView = viewInfoMap.find(viewId);
        if (itView == viewInfoMap.end())
        {
            LOG_ERR("No username found for viewId [" << viewId << "].");
            oss << "\"username\":\"Unknown\",";
        }
        else
        {
            oss << "\"userid\":\"" << JsonUtil::escapeJSONValue(itView->second.getUserId()) << "\",";
            const std::string username = itView->second.getUserName();
            oss << "\"username\":\"" << JsonUtil::escapeJSONValue(username) << "\",";
            if (!itView->second.getUserExtraInfo().empty())
                oss << "\"userextrainfo\":" << itView->second.getUserExtraInfo() << ',';
            const bool readonly = itView->second.isReadOnly();
            oss << "\"readonly\":\"" << readonly << "\",";
            const auto it = viewColorsMap.find(username);
            if (it != viewColorsMap.end())
            {
                color = it->second;
            }
        }

        oss << "\"color\":" << color;

        viewStrings[viewId] = oss.str();
    }

    // Broadcast updated viewinfo to all clients. Every view gets own userprivateinfo.
    for (const auto& it : _sessions)
    {
        std::ostringstream oss;
        oss << "viewinfo: [";

        for (const auto& viewId : viewIds)
        {
            oss << "{" << viewStrings[viewId];
            if (viewId == it.second->getViewId())
            {
                if (!it.second->getUserPrivateInfo().empty())
                {
                    std::string userPrivateInfo = it.second->getUserPrivateInfo();
                    std::initializer_list<std::string> keys = {
                        "SignatureCert",
                        "SignatureKey",
                        "SignatureCa",
                    };
                    replaceKeysWithPlaceholder(userPrivateInfo, keys);
                    oss << ",\"userprivateinfo\":" << userPrivateInfo;
                }
                if (!it.second->getServerPrivateInfo().empty())
                {
                    std::string serverPrivateInfo = it.second->getServerPrivateInfo();
                    std::initializer_list<std::string> keys = {
                        "ESignatureSecret",
                    };
                    replaceKeysWithPlaceholder(serverPrivateInfo, keys);
                    oss << ",\"serverprivateinfo\":" << serverPrivateInfo;
                }
            }
            oss << "},";
        }

        if (viewCount > 0)
            oss.seekp(-1, std::ios_base::cur); // Remove last comma.

        oss << ']';

        it.second->sendTextFrame(oss.str());
    }
}

std::shared_ptr<ChildSession> Document::findSessionByViewId(int viewId)
{
    for (const auto& it : _sessions)
    {
        if (it.second->getViewId() == viewId)
            return it.second;
    }

    return nullptr;
}

void Document::invalidateCanonicalId(const std::string& sessionId)
{
    auto it = _sessions.find(sessionId);
    if (it == _sessions.end())
    {
        LOG_ERR("Session [" << sessionId << "] not found");
        return;
    }
    std::shared_ptr<ChildSession> session = it->second;
    CanonicalViewId newCanonicalId = _sessions.createCanonicalId(getViewProps(session));
    if (newCanonicalId == session->getCanonicalViewId())
        return;
    session->setCanonicalViewId(newCanonicalId);
    std::string viewRenderedState = session->getViewRenderState();
    std::string stateName;
    if (!viewRenderedState.empty())
    {
        stateName = std::move(viewRenderedState);
    }
    else
    {
        stateName = "Empty";
    }
    std::string message = "canonicalidchange: viewid=" + std::to_string(session->getViewId()) +
        " canonicalid=" + std::to_string(to_underlying(newCanonicalId)) +
        " viewrenderedstate=" + stateName;
    session->sendTextFrame(message);
}

std::string Document::getViewProps(const std::shared_ptr<ChildSession>& session)
{
    return session->getWatermarkText() + "|" + session->getViewRenderState();
}

void Document::updateEditorSpeeds(int id, int speed)
{
    int maxSpeed = -1, fastestUser = -1;

    auto now = std::chrono::steady_clock::now();
    _lastUpdatedAt[id] = now;
    _speedCount[id] = speed;

    for (const auto& it : _sessions)
    {
        const std::shared_ptr<ChildSession>& session = it.second;
        int sessionId = session->getViewId();

        auto duration = (_lastUpdatedAt[id] - now);
        std::chrono::milliseconds::rep durationInMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        if (_speedCount[sessionId] != 0 && durationInMs > 5000)
        {
            _speedCount[sessionId] = session->getSpeed();
            _lastUpdatedAt[sessionId] = now;
        }
        if (_speedCount[sessionId] > maxSpeed)
        {
            maxSpeed = _speedCount[sessionId];
            fastestUser = sessionId;
        }
    }
    // 0 for preventing selection of the first always
    // 1 for preventing new users from directly becoming editors
    if (_editorId != fastestUser && (maxSpeed != 0 && maxSpeed != 1)) {
        if (!_editorChangeWarning && _editorId != -1)
        {
            _editorChangeWarning = true;
        }
        else
        {
            _editorChangeWarning = false;
            _editorId = fastestUser;
            for (const auto& it : _sessions)
                it.second->sendTextFrame("editor: " + std::to_string(_editorId));
        }
    }
    else
        _editorChangeWarning = false;
}


// Get the color value for all author names from the core
std::map<std::string, int> Document::getViewColors()
{
    LOKitHelper::ScopedString values(_loKitDocument->getCommandValues(".uno:TrackedChangeAuthors"));
    const std::string colorValues = std::string(values.get() == nullptr ? "" : values.get());

    std::map<std::string, int> viewColors;
    try
    {
        if (!colorValues.empty())
        {
            Poco::JSON::Parser parser;
            Poco::JSON::Object::Ptr root = parser.parse(colorValues).extract<Poco::JSON::Object::Ptr>();
            if (root->get("authors").type() == typeid(Poco::JSON::Array::Ptr))
            {
                Poco::JSON::Array::Ptr authorsArray = root->get("authors").extract<Poco::JSON::Array::Ptr>();
                for (const auto& authorVar : *authorsArray)
                {
                    const Poco::JSON::Object::Ptr& authorObj =
                        authorVar.extract<Poco::JSON::Object::Ptr>();
                    std::string authorName = authorObj->get("name").convert<std::string>();
                    int colorValue = authorObj->get("color").convert<int>();
                    viewColors[authorName] = colorValue;
                }
            }
        }
    }
    catch(const Exception& exc)
    {
        LOG_ERR("Poco Exception: " << exc.displayText() <<
                (exc.nested() ? " (" + exc.nested()->displayText() + ')' : ""));
    }

    return viewColors;
}

std::string Document::getDefaultTheme(const std::shared_ptr<ChildSession>& session) const
{
    bool darkTheme = session->getDarkTheme() == "true";
    return darkTheme ? "Dark" : "Light";
}

std::string Document::getDefaultBackgroundTheme(const std::shared_ptr<ChildSession>& session) const
{
    bool darkTheme = session->getDarkBackground() == "true";
    return darkTheme ? "Dark" : "Light";
}

std::shared_ptr<kit::Document> Document::load(const std::shared_ptr<ChildSession>& session,
                                              const std::string& renderOpts)
{
    const std::string sessionId = session->getId();

#ifdef _WIN32
    // For this to work with UNC paths, we need to use getDocURL() here, which is the original full
    // URL including the server. getJailedFilePath() ignores the server.
    const std::string& uri = session->getDocURL();
#else
    const std::string& uri = session->getJailedFilePath();
#endif
    const std::string& uriAnonym = session->getJailedFilePathAnonym();
    const std::string& userName = session->getUserName();
    const std::string& userNameAnonym = session->getUserNameAnonym();
    const std::string& docPassword = session->getDocPassword();
    const bool haveDocPassword = session->getHaveDocPassword();
    const std::string& lang = session->getLang();
    const std::string& deviceFormFactor = session->getDeviceFormFactor();
    const std::string& batchMode = session->getBatchMode();
    const std::string& enableMacrosExecution = session->getEnableMacrosExecution();
    const std::string& macroSecurityLevel = session->getMacroSecurityLevel();
    const std::string& clientVisibleArea = session->getInitialClientVisibleArea();
    const bool accessibilityState = session->getAccessibilityState();
    const std::string& userTimezone = session->getTimezone();
    const std::string& userPrivateInfo = session->getUserPrivateInfo();
    const std::string& docTemplate = session->getDocTemplate();
    const std::string& filterOption = session->getInFilterOption();

    if constexpr (!Util::isMobileApp())
        consistencyCheckFileExists(uri);

    std::string options;

    if (!filterOption.empty())
        options = filterOption;

    if (!lang.empty())
        options += ",Language=" + lang;

    if (!deviceFormFactor.empty())
        options += ",DeviceFormFactor=" + deviceFormFactor;

    if (!batchMode.empty())
        options += ",Batch=" + batchMode;

    if (!enableMacrosExecution.empty())
        options += ",EnableMacrosExecution=" + enableMacrosExecution;

    if (!macroSecurityLevel.empty())
        options += ",MacroSecurityLevel=" + macroSecurityLevel;

    if (!clientVisibleArea.empty())
        options += ",ClientVisibleArea=" + clientVisibleArea;

    if (!userTimezone.empty())
        options += ",Timezone=" + userTimezone;

    const std::string wopiCertDir = pathFromFileURL(session->getJailedFilePath() + ".certs");
    if (FileUtil::Stat(wopiCertDir).exists())
        ::setenv("LO_CERTIFICATE_AUTHORITY_PATH", wopiCertDir.c_str(), 1);

    if constexpr (!Util::isMobileApp())
    {
        // if ssl client verification was disabled in online for the wopi server,
        // and this is a https connection then also exempt that host from ssl host
        // verification in 'core'
        if (session->isDisableVerifyHost())
        {
            std::string scheme, host, port;
            if (net::parseUri(session->getDocURL(), scheme, host, port) && scheme == "https://")
                ::setenv("KIT_EXEMPT_VERIFY_HOST", host.c_str(), 1);
        }
    }

    std::string spellOnline = session->getSpellOnline();
    if (!_loKitDocument)
    {
        // This is the first time we are loading the document
        LOG_INF("Loading new document from URI: [" << uriAnonym << "] for session [" << sessionId << "].");

        _loKit->registerCallback(GlobalCallback, this);

        const int flags = KIT_FEATURE_DOCUMENT_PASSWORD
            | KIT_FEATURE_DOCUMENT_PASSWORD_TO_MODIFY
            | KIT_FEATURE_PART_IN_INVALIDATION_CALLBACK
            | KIT_FEATURE_NO_TILED_ANNOTATIONS
            | KIT_FEATURE_RANGE_HEADERS
            | KIT_FEATURE_VIEWID_IN_VISCURSOR_INVALIDATION_CALLBACK;
        _loKit->setOptionalFeatures(flags);

        std::string loadUri = uri;

        if (!docTemplate.empty())
        {
            // The template has been downloaded to 'uri'
            // But since the template might have a different format we temporarily
            // change the url to have the correct extension
            // It will be saved back to 'uri' in ChildSession once loaded
            Poco::URI pocoUri(uri), templateUri(docTemplate);
            Poco::Path newPath(pocoUri.getPath()), templatePath(templateUri.getPath());
            newPath.setExtension(templatePath.getExtension());
            if (::rename(pocoUri.getPath().c_str(), newPath.toString().c_str()) < 0)
                LOG_SYS("Failed to rename [" << pocoUri.getPath() << "] to [" << newPath.toString() << ']');
            pocoUri.setPath(newPath.toString());
            loadUri = pocoUri.toString();
        }

        // Save the provided password with us and the jailed url
        _haveDocPassword = haveDocPassword;
        _docPassword = docPassword;
        _jailedUrl = loadUri;
        _isDocPasswordProtected = false;

        const char* url = loadUri.c_str();
        LOG_DBG("Calling lokit::documentLoad(" << anonymizeUrl(url) << ", \"" << options << "\")");
        const auto start = std::chrono::steady_clock::now();
        _loKitDocument.reset(_loKit->documentLoad(url, options.c_str()));
#ifdef __ANDROID__
        _loKitDocumentForAndroidOnly = _loKitDocument;
        {
            std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
            auto docBrokerIt = DocBrokers.find(_docKey);
            assert(docBrokerIt != DocBrokers.end());
            _documentBrokerForAndroidOnly = docBrokerIt->second;
        }
#endif
        const auto duration = std::chrono::steady_clock::now() - start;
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        LOG_DBG("Returned lokit::documentLoad(" << anonymizeUrl(url) << ") in " << elapsed);
#if defined(IOS) || defined(MACOS) || defined(_WIN32) || defined(QTAPP)
        DocumentData::get(_mobileAppDocId).loKitDocument = _loKitDocument.get();
        {
            std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
            auto docBrokerIt = DocBrokers.find(_docKey);
            assert(docBrokerIt != DocBrokers.end());
            DocumentData::get(_mobileAppDocId).docBroker = docBrokerIt->second;
        }
#endif
        if (!_loKitDocument || !_loKitDocument->get())
        {
            LOG_ERR("Failed to load: " << uriAnonym << ", error: " << _loKit->getError());

            // Checking if wrong password or no password was reason for failure.
            if (_isDocPasswordProtected)
            {
                LOG_INF("Document [" << uriAnonym << "] is password protected.");
                if (!_haveDocPassword)
                {
                    LOG_INF("No password provided for password-protected document [" << uriAnonym << "].");
                    std::string passwordFrame = "passwordrequired:";
                    if (_docPasswordType == DocumentPasswordType::ToView)
                        passwordFrame += "to-view";
                    else if (_docPasswordType == DocumentPasswordType::ToModify)
                        passwordFrame += "to-modify";
                    session->sendTextFrameAndLogError("error: cmd=load kind=" + passwordFrame);
                }
                else
                {
                    LOG_INF("Wrong password for password-protected document [" << uriAnonym << "].");
                    session->sendTextFrameAndLogError("error: cmd=load kind=wrongpassword");
                }
                return nullptr;
            }

            session->sendTextFrameAndLogError("error: cmd=load kind=faileddocloading");
            session->shutdownNormal();

            LOG_FTL("Failed to load the document. Setting TerminationFlag");
            SigUtil::setTerminationFlag();
            return nullptr;
        }

        // Only save the options on opening the document.
        // No support for changing them after opening a document.
        _renderOpts = renderOpts;
    }
    else
    {
        LOG_INF("Document with url [" << uriAnonym << "] already loaded. Need to create new view for session [" << sessionId << "].");

        // Check if this document requires password
        if (_isDocPasswordProtected)
        {
            if (!haveDocPassword)
            {
                std::string passwordFrame = "passwordrequired:";
                if (_docPasswordType == DocumentPasswordType::ToView)
                    passwordFrame += "to-view";
                else if (_docPasswordType == DocumentPasswordType::ToModify)
                    passwordFrame += "to-modify";
                session->sendTextFrameAndLogError("error: cmd=load kind=" + passwordFrame);
                return nullptr;
            }
            else if (docPassword != _docPassword)
            {
                session->sendTextFrameAndLogError("error: cmd=load kind=wrongpassword");
                return nullptr;
            }
        }

        LOG_INF("Creating view to url [" << uriAnonym << "] for session [" << sessionId << "] with " << options << '.');
        _loKitDocument->createView(options.c_str());
        LOG_TRC("View to url [" << uriAnonym << "] created.");

        switch (_loKitDocument->getDocumentType())
        {
        case KIT_DOCTYPE_TEXT:
        case KIT_DOCTYPE_SPREADSHEET:
            // writer and calc can have different spell checking settings per view, so use this users
            // preference
            break;
        case KIT_DOCTYPE_PRESENTATION:
        case KIT_DOCTYPE_DRAWING:
        default:
            // impress/draw currently cannot, so use the current document state
            // so simply joining doesn't toggle that shared spelling state
            LOKitHelper::ScopedString viewRenderState(_loKitDocument->getCommandValues(".uno:ViewRenderState"));
            if (viewRenderState.get())
            {
                StringVector tokens(StringVector::tokenize(viewRenderState.get(), strlen(viewRenderState.get()), ';'));
                spellOnline = tokens[0] == "S" ? "true" : "false";
            }
            break;
        }
    }
    std::string theme = getDefaultTheme(session);

    std::string backgroundTheme = getDefaultBackgroundTheme(session);

    // Avoid logging userPrivateInfo till it's not anonymized.
    LOG_INF("Initializing for rendering session [" << sessionId << "] on document url [" <<
            anonymizeUrl(_url) << "] with: [" << makeRenderParams(_renderOpts, userNameAnonym, spellOnline, theme, backgroundTheme, "") << "].");

    // initializeForRendering() should be called before
    // registerCallback(), as the previous creates a new view in Impress.
    const std::string renderParams = makeRenderParams(_renderOpts, userName, spellOnline, theme, backgroundTheme, userPrivateInfo);

    _loKitDocument->initializeForRendering(renderParams.c_str());

    const int viewId = _loKitDocument->getView();
    session->setViewId(viewId);
    if (viewId < 0)
    {
        LOG_ERR("Failed to load view into document url [" << anonymizeUrl(_url) << "] for session ["
                                                          << sessionId << ']');
        return nullptr;
    }

    _sessionUserInfo[viewId] = UserInfo(session->getViewUserId(), session->getViewUserName(),
                                        session->getViewUserExtraInfo(), session->getViewUserPrivateInfo(),
                                        session->isReadOnly());

    if (!lang.empty())
    {
        _loKitDocument->setViewLanguage(viewId, lang.c_str());
    }
    _loKitDocument->setViewTimezone(viewId, userTimezone.c_str());
    _loKitDocument->setAccessibilityState(viewId, accessibilityState);
    if (session->isReadOnly())
    {
        _loKitDocument->setViewReadOnly(viewId, true);
        if (session->isAllowChangeComments())
        {
            _loKitDocument->setAllowChangeComments(viewId, true);
        }
        if (session->isAllowManageRedlines())
        {
            _loKitDocument->setAllowManageRedlines(viewId, true);
        }
    }

    // viewId's monotonically increase, and CallbackDescriptors are never freed.
    _viewIdToCallbackDescr.emplace(viewId,
                                   std::unique_ptr<CallbackDescriptor>(new CallbackDescriptor({ this, viewId })));
    _loKitDocument->registerCallback(ViewCallback, _viewIdToCallbackDescr[viewId].get());

    const int viewCount = _loKitDocument->getViewsCount();
    LOG_INF("Document url [" << anonymizeUrl(_url) << "] for session [" <<
            sessionId << "] loaded view [" << viewId << "]. Have " <<
            viewCount << " view" << (viewCount != 1 ? "s." : "."));

    session->initWatermark();

    LOKitHelper::ScopedString viewRenderState(_loKitDocument->getCommandValues(".uno:ViewRenderState"));
    if (viewRenderState.get())
    {
        session->setViewRenderState(viewRenderState.get());
    }

    invalidateCanonicalId(session->getId());

#ifdef _WIN32
    load_next_document();
#endif
    return _loKitDocument;
}

int Document::getViewsCount() const
{
    return _loKitDocument ? _loKitDocument->getViewsCount() : 0;
}

bool Document::forwardToChild(const std::string_view prefix, const std::vector<char>& payload)
{
    assert(Util::isFuzzing() || payload.size() > prefix.size());

    // Remove the prefix and trim.
    std::size_t index = prefix.size();
    for ( ; index < payload.size(); ++index)
    {
        if (payload[index] != ' ')
        {
            break;
        }
    }

    const char* data = payload.data() + index;
    std::size_t size = payload.size() - index;

    std::string name;
    std::string sessionId;
    if (COOLProtocol::parseNameValuePair(prefix, name, sessionId, '-') && name == "child")
    {
        const auto it = _sessions.find(sessionId);
        if (it != _sessions.end())
        {
            std::shared_ptr<ChildSession> session = it->second;

            constexpr std::string_view disconnect("disconnect");
            if (size == disconnect.size() &&
                strncmp(data, disconnect.data(), disconnect.size()) == 0)
            {
                if(session->getViewId() == _editorId) {
                    _editorId = -1;
                }
                LOG_INF("Removing ChildSession [" << sessionId << "].");

                // Tell them we're going quietly.
                session->sendTextFrame("disconnected:");

                _sessions.erase(it);
                const std::size_t count = _sessions.size();
                LOG_DBG("Have " << count << " child" << (count == 1 ? "" : "ren") <<
                        " after removing ChildSession [" << sessionId << "].");

                _deltaGen->setSessionCount(count);

                _sessionUserInfo[session->getViewId()].setDisconnected();

                // No longer needed, and allow session dtor to take it.
                session.reset();
                return true;
            }

            // No longer needed, and allow the handler to take it.
            if (session)
            {
                std::vector<char> vect(size);
                vect.assign(data, data + size);

                // TODO this is probably wrong...
                session->handleMessage(vect);
                return true;
            }
        }

        std::string abbrMessage;
#ifndef BUILDING_TESTS
        if (Anonymizer::enabled())
        {
            abbrMessage = "...";
        }
        else
#endif
        {
            abbrMessage = getAbbreviatedMessage(data, size);
        }

        if constexpr (!Util::isFuzzing())
            LOG_ERR("Child session [" << sessionId
                                      << "] not found to forward message: " << abbrMessage);
    }
    else if constexpr (!Util::isFuzzing())
    {
        LOG_ERR("Failed to parse prefix of forward-to-child message: " << prefix);
    }

    return false;
}

/* static */ std::string Document::makeRenderParams(const std::string& renderOpts, const std::string& userName,
                                                    const std::string& spellOnline, const std::string& theme,
                                                    const std::string& backgroundTheme,
                                                    const std::string& userPrivateInfo)
{
    Object::Ptr renderOptsObj;

    // Fill the object with renderoptions, if any
    if (!renderOpts.empty())
    {
        Parser parser;
        Poco::Dynamic::Var var = parser.parse(renderOpts);
        renderOptsObj = var.extract<Object::Ptr>();
    }
    else
    {
        renderOptsObj = new Object();
    }

    Object::Ptr userPrivateInfoObj;
    if (!userPrivateInfo.empty())
    {
        Parser parser;
        Poco::Dynamic::Var var = parser.parse(userPrivateInfo);
        try
        {
            userPrivateInfoObj = var.extract<Object::Ptr>();
        }
        catch (const Poco::BadCastException& exception)
        {
            LOG_DBG("user private data is not a dictionary: " << exception.what());
        }
    }
    if (!userPrivateInfoObj)
    {
        userPrivateInfoObj = new Object();
    }

    // Append name of the user, if any, who opened the document to rendering options
    if (!userName.empty())
    {
        // userName must be decoded already.
        renderOptsObj->set(".uno:Author", makePropertyValue("string", userName));
    }

    // Extract settings relevant as view options from userPrivateInfo.
    std::string signatureCa;
    JsonUtil::findJSONValue(userPrivateInfoObj, "SignatureCa", signatureCa);
    if (!signatureCa.empty())
    {
        renderOptsObj->set(".uno:SignatureCa", makePropertyValue("string", signatureCa));
    }

    // By default we enable spell-checking, unless it's disabled explicitly.
    if (!spellOnline.empty())
    {
        const bool set = (spellOnline != "false");
        renderOptsObj->set(".uno:SpellOnline", makePropertyValue("boolean", set));
    }

    if (!theme.empty())
        renderOptsObj->set(".uno:ChangeTheme", makePropertyValue("string", theme));

    if (!backgroundTheme.empty())
        renderOptsObj->set(".uno:InvertBackground", makePropertyValue("string", backgroundTheme));

    if (renderOptsObj)
    {
        std::ostringstream ossRenderOpts;
        renderOptsObj->stringify(ossRenderOpts);
        return ossRenderOpts.str();
    }

    return std::string();
}

TilePrioritizer::Priority Document::getTilePriority(const TileDesc &desc) const
{
    TilePrioritizer::Priority maxPrio = TilePrioritizer::Priority::NONE;
    const auto canonicalViewId = desc.getCanonicalViewId();

    assert(_sessions.size() > 0);
    for (const auto& [sessionName, session] : _sessions)
    {
        // only interested in sessions that match our viewId
        if (session->getCanonicalViewId() != canonicalViewId)
            continue;

        maxPrio = std::max(maxPrio, session->getTilePriority(desc));
    }

    if (maxPrio == TilePrioritizer::Priority::NONE)
    {
        // This can be highly noisy when a view is removed
        // but we have a long back-log of tiles to deliver.
        static CanonicalViewId lastViewId = CanonicalViewId::Invalid;
        if (canonicalViewId != lastViewId)
        {
            lastViewId = canonicalViewId;
            LOG_WRN("No sessions match this viewId " << canonicalViewId);
        }
    }

    // LOG_TRC("Priority for tile " << desc.generateID() << " is " << maxPrio);
    return maxPrio;
}

std::vector<TilePrioritizer::ViewIdInactivity> Document::getViewIdsByInactivity() const
{
    std::vector<TilePrioritizer::ViewIdInactivity> viewIds;

    const auto now = std::chrono::steady_clock::now();

    assert(_sessions.size() > 0);
    for (const auto& it : _sessions)
    {
        const std::shared_ptr<ChildSession> &session = it.second;

        double sessionInactivity = session->getInactivityMS(now);
        CanonicalViewId viewId = session->getCanonicalViewId();

        auto found = std::find_if(viewIds.begin(), viewIds.end(),
                                  [viewId](const auto& entry)->bool {
                                    return entry.first == viewId;
                                  });
        if (found == viewIds.end())
            viewIds.emplace_back(viewId, sessionInactivity);
        else if (sessionInactivity < found->second)
            found->second = sessionInactivity;
    }

    std::sort(viewIds.begin(), viewIds.end(), [](const auto& a, const auto& b) {
                                                return a.second < b.second;
                                              });

    return viewIds;
}

bool Document::processInputEnabled() const
{
    bool enabled = !_websocketHandler || _websocketHandler->processInputEnabled();
    if (!enabled)
        LOG_TRC("Document - not processing input");
    return enabled;
}

void Document::drainCallbacks()
{
    KitQueue::Callback cb;

    LOG_TRC("drainCallbacks with " << _queue->callbackSize() << " items");

    while (_queue && _queue->getCallback(cb))
    {
        if (_stop || SigUtil::getTerminationFlag())
        {
            LOG_INF("_stop or TerminationFlag is set, breaking Document::drainCallbacks");
            break;
        }

        LOG_TRC("Kit handling callback " << cb);

        int viewId = cb._view;
        bool broadcast = cb._view == -1;

        const int type = cb._type;
        const std::string &payload = cb._payload;

        // Forward the callback to the same view, demultiplexing is done by the CollaboraOffice core.
        bool isFound = false;
        for (const auto& it : _sessions)
        {
            ChildSession& session = *it.second;
            if (broadcast || (!broadcast && (session.getViewId() == viewId)))
            {
                if (!session.isCloseFrame())
                {
                    isFound = true;
                    session.loKitCallback(type, payload);
                }
                else
                {
                    LOG_ERR("Session-thread of session ["
                            << session.getId() << "] for view [" << viewId
                            << "] is not running. Dropping ["
                            << kitCallbackTypeToString(type) << "] payload ["
                            << COOLProtocol::getAbbreviatedMessage(payload)
                            << ']');
                }

                if (!broadcast)
                    break;
            }
        }
        if (!isFound)
            LOG_ERR("Document::ViewCallback. Session [" << viewId <<
                    "] is no longer active to process [" << kitCallbackTypeToString(type) <<
                    "] [" << COOLProtocol::getAbbreviatedMessage(payload) <<
                    "] message to Master Session.");
    }

    if (_websocketHandler)
        _websocketHandler->flush();
}

void Document::drainQueue()
{
#if !WASMAPP
    if (!Util::isMobileApp() && UnitKit::get().filterDrainQueue())
    {
        LOG_TRC("Filter disabled drainQueue");
        return;
    }
#endif

    try
    {
        if (hasCallbacks())
            drainCallbacks();

        if (hasQueueItems())
            LOG_TRC("drainQueue with " << _queue->size() <<
                    " items: " << (processInputEnabled() ? "processing" : "blocked") );

        // FIXME: do we really want to process all of these items ?
        while (processInputEnabled() && hasQueueItems())
        {
            if (_stop || SigUtil::getTerminationFlag())
            {
                LOG_INF("_stop or TerminationFlag is set, breaking Document::drainQueue of loop");
                _queue->clearTileQueue();
                _deltaPool.stop();
                break;
            }

            const KitQueue::Payload input = _queue->pop();

            LOG_TRC("Kit handling queue message: " << COOLProtocol::getAbbreviatedMessage(input));

            const StringVector tokens = StringVector::tokenize(input.data(), input.size());

            if (tokens.equals(0, "eof"))
            {
                LOG_INF("Received EOF. Finishing.");
                break;
            }
            else if (tokens.equals(0, "tile") || tokens.equals(0, "tilecombine"))
            {
                assert(Util::isFuzzing() &&
                       "Should not have incoming tile requests in message queue");
            }
            else if (tokens.startsWith(0, "child-"))
            {
                forwardToChild(tokens[0], input);
            }
            else if (tokens.equals(0, "callback"))
            {
                assert(Util::isFuzzing() && "callbacks cannot now appear on the incoming queue");
            }
            else
            {
                LOG_ERR("Unexpected request: [" << COOLProtocol::getAbbreviatedMessage(input) << "].");
            }
        }

        if (!_sessions.empty() && canRenderTiles())
        {
            // Priority for tiles of visible part that intersect with an active viewport
            TilePrioritizer::Priority prio = TilePrioritizer::Priority::VERYHIGH;
            while (!_queue->isTileQueueEmpty() && prio >= TilePrioritizer::Priority::VERYHIGH)
            {
                TileCombined tileCombined = _queue->popTileQueue(prio);
                LOG_TRC("Tile priority is " << static_cast<int>(prio) << " for " << tileCombined.serialize());

                renderTiles(tileCombined);
            }
            // if priority is low - do one render, then process more events.
        }
    }
    catch (const std::exception& exc)
    {
        LOG_FTL("drainQueue: Exception: " << exc.what());
        if constexpr (!Util::isMobileApp())
            flushAndExit(EX_SOFTWARE);
    }
    catch (...)
    {
        LOG_FTL("drainQueue: Unknown exception");
        if constexpr (!Util::isMobileApp())
            flushAndExit(EX_SOFTWARE);
    }
}

/// Return access to the kit::Document instance.
std::shared_ptr<kit::Document> Document::getLOKitDocument()
{
    if (!_loKitDocument)
    {
        LOG_ERR("Document [" << _docKey << "] is not loaded.");
        throw std::runtime_error("Document " + _docKey + " is not loaded.");
    }

    return _loKitDocument;
}

void Document::postForceModifiedCommand(bool modified)
{
    std::string args = "{ \"Modified\": { \"type\": \"boolean\", ";
    args += "\"value\": \"";
    args += (modified ? "true" : "false");
    args += "\" } }";

    LOG_TRC("post force modified command: .uno:Modified " << args);

    // Interestingly this seems not to notify the modified state change.
    getLOKitDocument()->postUnoCommand(
        ".uno:Modified", args.c_str(),
        false /* avoid an un-necessary unocommandresult */);
}

void Document::forceDocUnmodifiedForBgSave(int viewId)
{
    LOG_TRC("force document unmodified from state " << name(_modified));
    if (_modified == ModifiedState::Modified)
    {
        getLOKitDocument()->setView(viewId);

        SigUtil::addActivity("Force clear modified");
        _modified = ModifiedState::UnModifiedButSaving;
        // but tell the core we are not modified to track real changes
        postForceModifiedCommand(false);
    }
}

void Document::updateModifiedOnFailedBgSave()
{
    if (_modified == ModifiedState::UnModifiedButSaving)
    {
        SigUtil::addActivity("Force re-modified");
        _modified = ModifiedState::Modified;
        postForceModifiedCommand(true);
    }
}

void Document::notifySyntheticUnmodifiedState()
{
    // no need to change core state that happened earlier
    if (_modified == ModifiedState::UnModifiedButSaving)
    {
        LOG_TRC("document was not modified while background saving; sending synthetic "
                ".uno:ModifiedStatus=false");
        _modified = ModifiedState::UnModified;
        notifyAll("statechanged: .uno:ModifiedStatus=false");
    }
}

bool Document::trackDocModifiedState(const std::string &stateChanged)
{
    bool filter = false;

    const StringVector tokens(StringVector::tokenize(stateChanged, '='));
    const bool modified = tokens.size() > 1 && tokens.equals(1, "true");
    ModifiedState newState = _modified;
    // NB. since 'modified' state is (oddly) notified per view we get
    // several duplicate transitions from state A -> A again.
    switch (_modified) {
    case ModifiedState::Modified:
        if (!modified)
            newState = ModifiedState::UnModified;
        // else duplicate
        break;
    // Only present in background save mode
    case ModifiedState::UnModifiedButSaving:
        if (modified)
        {
            // now we're really modified
            newState = ModifiedState::Modified;
        }
        else // ignore being notified of our own force unmodification.
        {
            LOG_TRC("Ignore self generated unmodified notification");
            filter = true;
        }
        break;
    case ModifiedState::UnModified:
        if (modified)
            newState = ModifiedState::Modified;
        // else duplicate
        break;
    }

    if (_modified != newState)
    {
        LOG_TRC("Transition modified state from " << name(_modified) << " to " << name(newState));
        _modified = newState;
    }
    else
        LOG_TRC("Modified state remains " << name(_modified) << " after " << stateChanged);

    return filter;
}

void Document::disableBgSave(const std::string &reason)
{
    LOG_WRN("Disabled background save " << reason);
    _isBgSaveDisabled = true;
}

/// Stops theads, flushes buffers, and exits the process.
void Document::flushAndExit(int code)
{
    flushTraceEventRecordings();
    _deltaPool.stop();
    if constexpr (!Util::isKitInProcess())
        Util::forcedExit(code);
    else
        SigUtil::setTerminationFlag();
}

void Document::dumpState(std::ostream& oss)
{
    oss << "Kit Document:\n"
        << "\n\tpid: " << ProcUtil::getProcessId() << "\n\tstop: " << _stop
        << "\n\tjailId: " << _jailId << "\n\tdocKey: " << _docKey << "\n\tdocId: " << _docId
        << "\n\turl: " << _url << "\n\tobfuscatedFileId: " << _obfuscatedFileId
        << "\n\tjailedUrl: " << anonymizeUrl(_jailedUrl) << "\n\trenderOpts: " << _renderOpts
        << "\n\thaveDocPassword: " << _haveDocPassword // not the pwd itself
        << "\n\tisDocPasswordProtected: " << _isDocPasswordProtected
        << "\n\tdocPasswordType: " << (int)_docPasswordType << "\n\teditorId: " << _editorId
        << "\n\teditorChangeWarning: " << _editorChangeWarning
        << "\n\tmobileAppDocId: " << _mobileAppDocId
        << "\n\tinputProcessingEnabled: " << processInputEnabled()
        << "\n\tduringLoad: " << _duringLoad << "\n\tmodified: " << name(_modified)
        << "\n\tbgSaveProc: " << _isBgSaveProcess << "\n\tbgSaveDisabled: " << _isBgSaveDisabled;
    if (!_isBgSaveProcess)
        oss << "\n\tbgSavesOnging: "<< _bgSavesOngoing;

    std::string smap;
    if (const ssize_t size = FileUtil::readFile("/proc/self/smaps_rollup", smap); size <= 0)
        oss << "\n\tsmaps_rollup: <unavailable>";
    else
        oss << "\n\tsmaps_rollup: " << Util::replace(std::move(smap), "\n", "\n\t");
    oss << '\n';

    // dumpState:
    // TODO: _websocketHandler - but this is an odd one.
    _queue->dumpState(oss);
    oss << "\tviewIdToCallbackDescr:";
    for (const auto &it : _viewIdToCallbackDescr)
    {
        oss << "\n\t\tviewId: " << it.first
            << " editorId: " << it.second->getDoc()->getEditorId()
            << " mobileAppDocId: " << it.second->getDoc()->getMobileAppDocId();
    }
    oss << '\n';

    _deltaPool.dumpState(oss);
    _sessions.dumpState(oss);

    _deltaGen->dumpState(oss);

    oss << "\tlastUpdatedAt:";
    for (const auto &it : _lastUpdatedAt)
    {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            it.second.time_since_epoch()).count();
        oss << "\n\t\tviewId: " << it.first
            << " last update time(ms): " << ms;
    }
    oss << '\n';

    oss << "\tspeedCount:";
    for (const auto &it : _speedCount)
    {
        oss << "\n\t\tviewId: " << it.first
            << " speed: " << it.second;
    }
    oss << "\n";

    /// For showing disconnected user info in the doc repair dialog.
    oss << "\tsessionUserInfo:";
    for (const auto &it : _sessionUserInfo)
    {
        oss << "\n\t\tviewId: " << it.first
            << " userId: " << Anonymizer::anonymize(it.second.getUserId())
            << " userName: " << Anonymizer::anonymize(it.second.getUserName())
            << " userExtraInfo: " << it.second.getUserExtraInfo()
            << " readOnly: " << it.second.isReadOnly()
            << " connected: " << it.second.isConnected();
    }
    oss << '\n';

    char *pState = nullptr;
    _loKit->dumpState("", &pState);
    oss << "lok state:\n";
    if (pState) {
        std::string stateStr(pState);
        std::string fileId = Uri::getFilenameFromURL(Uri::decode(_jailedUrl));
        Util::replaceAllSubStr(stateStr, fileId, _obfuscatedFileId);
        oss << stateStr;
    }
    oss << '\n';
}

#if !defined BUILDING_TESTS && !MOBILEAPP && !LIBFUZZER

// When building the fuzzer we link COOLWSD.cpp into the same executable so the
// Protected::emitOneRecording() there gets used. When building the unit tests the one in
// TraceEvent.cpp gets used.

static std::mutex traceEventLock;
static std::vector<std::string> traceEventRecords[2];

void flushTraceEventRecordings()
{
    std::unique_lock<std::mutex> lock(traceEventLock);

    for (size_t n = 0; n < 2; ++n)
    {
        std::vector<std::string> &r = traceEventRecords[n];

        if (r.empty())
            continue;

        std::size_t totalLength = 32; // Provision for the command name.
        for (const auto& i: r)
            totalLength += i.length();

        std::string recordings;
        recordings.reserve(totalLength);

        recordings.append(n == 0 ? "forcedtraceevent: \n" : "traceevent: \n");
        for (const auto& i: r)
            recordings += i;

        singletonDocument->sendTextFrame(recordings);
        r.clear();
    }
}

static void addRecording(const std::string &recording, bool force)
{
    // This can be called before the config system is initialized. Guard against that, as calling
    // ConfigUtil::getBool() would cause an assertion failure.

    static bool configChecked = false;
    static bool traceEventsEnabled;
    if (!configChecked && ConfigUtil::isInitialized())
    {
        traceEventsEnabled = ConfigUtil::getBool("trace_event[@enable]", false);
        configChecked = true;
    }

    if (configChecked && !traceEventsEnabled)
        return;

    // catch if this gets called in the ForKit process & skip.
    if (singletonDocument == nullptr)
        return;

    if (!TraceEvent::isRecordingOn() && !force)
        return;

    std::unique_lock<std::mutex> lock(traceEventLock);

    traceEventRecords[force ? 0 : 1].push_back(recording + "\n");
}

#if !defined(QTAPP) && !defined(MACOS) && !defined(_WIN32) // ie. normal server
void TraceEvent::emitOneRecordingIfEnabled(const std::string &recording)
{
    addRecording(recording, true);
}

void TraceEvent::emitOneRecording(const std::string &recording)
{
    addRecording(recording, false);
}
#endif // !defined(QTAPP) && !defined(MACOS) && !defined(_WIN32)

#else

void flushTraceEventRecordings()
{
}

#endif

#ifdef __ANDROID__

std::shared_ptr<kit::Document> Document::_loKitDocumentForAndroidOnly = std::shared_ptr<kit::Document>();
std::weak_ptr<DocumentBroker> Document::_documentBrokerForAndroidOnly;

std::shared_ptr<kit::Document> getLOKDocumentForAndroidOnly()
{
    return Document::_loKitDocumentForAndroidOnly;
}

std::shared_ptr<DocumentBroker> getDocumentBrokerForAndroidOnly()
{
    return Document::_documentBrokerForAndroidOnly.lock();
}

#endif

KitSocketPoll::KitSocketPoll() : SocketPoll("kit")
{
#if defined(IOS) || defined(QTAPP) || defined(MACOS) || defined(_WIN32)
    termination = std::make_shared<KitSocketPoll::TerminationData>();
    termination->flag = false;
#endif
    mainPoll = this;
}

KitSocketPoll::~KitSocketPoll()
{
    // Just to make it easier to set a breakpoint
    mainPoll = nullptr;
}

void KitSocketPoll::dumpGlobalState(std::ostream& oss) // static
{
    if (mainPoll)
    {
        if (!mainPoll->_document)
            oss << "KitSocketPoll: no doc\n";
        else
        {
            mainPoll->_document->dumpState(oss);
            mainPoll->dumpState(oss);
        }
    }
    else
        oss << "KitSocketPoll: none\n";
}

std::shared_ptr<KitSocketPoll> KitSocketPoll::create() // static
{
    std::shared_ptr<KitSocketPoll> result(new KitSocketPoll());

#if defined(IOS) || defined(QTAPP) || defined(MACOS) || defined(_WIN32)
    {
        std::unique_lock<std::mutex> lock(KSPollsMutex);
        KSPolls.push_back(result);
    }
    KitSocketPoll::KSPollsCV.notify_all();
#endif
    return result;
}

/* static */ void KitSocketPoll::cleanupChildProcess()
{
    mainPoll->closeAllSockets();
    mainPoll->createWakeups();
}

// process pending message-queue events.
void KitSocketPoll::drainQueue()
{
    SigUtil::checkDumpGlobalState(dump_kit_state);

    if (_document)
        _document->drainQueue();
}

// called from inside poll, inside a wakeup
void KitSocketPoll::wakeupHook() { _pollEnd = std::chrono::steady_clock::now(); }

// a COKit compatible poll function merging the functions.
// returns the number of events signalled
int KitSocketPoll::kitPoll(int timeoutMicroS)
{
    ProfileZone profileZone("KitSocketPoll::kitPoll");

    if (SigUtil::getTerminationFlag())
    {
        LOG_TRC("Termination of unipoll mainloop flagged");
        return -1;
    }

#if ENABLE_DEBUG
#if !MOBILEAPP
    auto &unitKit = UnitKit::get();
    if (unitKit.isFinished())
    {
        static bool sentResult = false;
        if (!sentResult && singletonDocument)
        {
            LOG_TRC("Sending unit test result");
            singletonDocument->sendTextFrame(unitKit.getResultMessage());
            sentResult = true;
        }
    }
#endif

    static std::atomic<int> reentries = 0;
    static int lastWarned = 1;
    ReEntrancyGuard guard(reentries);
    if (reentries != lastWarned)
    {
        LOG_ERR("non-async dialog triggered");
#if !MOBILEAPP
        if (singletonDocument && lastWarned < reentries)
            singletonDocument->alertNotAsync();
#endif
        lastWarned = reentries;
    }
#endif

    // The maximum number of extra events to process beyond the first.
    int maxExtraEvents = 15;
    int eventsSignalled = 0;

    auto startTime = std::chrono::steady_clock::now();

    if (timeoutMicroS < 0)
    {
        // Flush at most 1 + maxExtraEvents, or return when nothing left.
        while (poll(std::chrono::microseconds::zero()) > 0 && maxExtraEvents-- > 0)
            ++eventsSignalled;
    }
    else
    {
        // Flush at most maxEvents+1, or return when nothing left.
        _pollEnd = startTime + std::chrono::microseconds(timeoutMicroS);
        do
        {
            int realTimeout = timeoutMicroS;
            if (_document && _document->needsQuickPoll())
                realTimeout = 0;

            if (poll(std::chrono::microseconds(realTimeout)) <= 0)
                break;

            const auto now = std::chrono::steady_clock::now();
            drainQueue();

            timeoutMicroS =
                std::chrono::duration_cast<std::chrono::microseconds>(_pollEnd - now).count();
            ++eventsSignalled;
        } while (timeoutMicroS > 0 && !SigUtil::getTerminationFlag() && maxExtraEvents-- > 0);
    }

    drainQueue();

    if (_document)
        _document->trimAfterInactivity();

    if constexpr (!Util::isMobileApp())
    {
        flushTraceEventRecordings();

        if (_document && _document->purgeSessions() == 0)
        {
            LOG_INF("Last session discarded. Setting TerminationFlag");
            SigUtil::setTerminationFlag();
            return -1;
        }
    }
    // Report the number of events we processed.
    return eventsSignalled;
}

// unusual COKit event from another thread, push into our loop to process.
bool KitSocketPoll::pushToMainThread(COKitCallback callback, int type,
                                     const char* p, void* data) // static
{
    if (mainPoll && mainPoll->getThreadOwner() != ProcUtil::getThreadId())
    {
        LOG_TRC("Unusual push callback to main thread");
        std::shared_ptr<std::string> copy;
        if (p)
            copy = std::make_shared<std::string>(p, strlen(p));
        mainPoll->addCallback([callback, type, data, copy = std::move(copy)] {
            LOG_TRC("Unusual process callback in main thread");
            callback(type, copy ? copy->c_str() : nullptr, data);
        });
        return true;
    }
    return false;
}

KitSocketPoll *KitSocketPoll::mainPoll = nullptr;

bool pushToMainThread(COKitCallback cb, int type, const char *p, void *data)
{
    return KitSocketPoll::pushToMainThread(cb, type, p, data);
}

#if defined(IOS) || defined(QTAPP) || defined(MACOS) || defined(_WIN32)

std::mutex KitSocketPoll::KSPollsMutex;
std::condition_variable KitSocketPoll::KSPollsCV;
std::vector<std::weak_ptr<KitSocketPoll>> KitSocketPoll::KSPolls;

#endif

void documentViewCallback(const int type, const char* payload, void* data)
{
    Document::ViewCallback(type, payload, data);
}

#ifndef BUILDING_TESTS

namespace
{

/// Called by COKit main-loop the central location for data processing.
int pollCallback([[maybe_unused]] void* data, int timeoutUs)
{
    if (!Util::isMobileApp())
        UnitKit::get().preKitPollCallback();

    if (timeoutUs < 0)
        timeoutUs = SocketPoll::DefaultPollTimeoutMicroS.count();
#if !defined(IOS) && !defined(QTAPP) && !defined(MACOS) && !defined(_WIN32)
    if (!data)
        return 0;
    else
        return reinterpret_cast<KitSocketPoll*>(data)->kitPoll(timeoutUs);
#else
    std::unique_lock<std::mutex> lock(KitSocketPoll::KSPollsMutex);
    std::vector<std::shared_ptr<KitSocketPoll>> v;
    v.reserve(KitSocketPoll::KSPolls.size());
    for (const auto &i : KitSocketPoll::KSPolls)
    {
        if (auto p = i.lock())
            v.push_back(std::move(p));
    }

    if (v.empty())
    {
        // Remove any stale elements from KitSocketPoll::KSPolls and
        // block until an element is added to KitSocketPoll::KSPolls
        KitSocketPoll::KSPolls.clear();
        KitSocketPoll::KSPollsCV.wait(lock, []{ return KitSocketPoll::KSPolls.size(); });
        return 0;
    }

    lock.unlock();

    // Non-blocking poll on all kits
    bool anyPollHadEvents = false;
    for (const auto &p : v) {
        // deliberately kitPoll(0) - returns right away
        if (p->kitPoll(0) > 0)
            anyPollHadEvents = true;
    }

    // If no poll had events, block until any fake-socket activity,
    // or until the global condition variable (theCV) timeout expires
    if (!anyPollHadEvents)
        fakeSocketWaitAny(timeoutUs);

    // We never want to exit the main loop
    return 0;
#endif
}

// Do we have any pending input events from coolwsd ?
bool anyInputCallback(void* data, int mostUrgentPriority)
{
    if (!data)
        return false;

    return reinterpret_cast<KitSocketPoll*>(data)->kitHasAnyInput(mostUrgentPriority);
}

} // namespace

bool KitSocketPoll::kitHasAnyInput([[maybe_unused]] int mostUrgentPriority) {
#if !defined(IOS) && !defined(QTAPP) && !defined(MACOS) && !defined(_WIN32)
    const std::shared_ptr<Document>& document = getDocument();

    if (document)
    {
        if (document->isLoaded())
        {
            // Check if core has high-priority tasks in which case we don't interrupt.
            std::shared_ptr<kit::Document> kitDocument = document->getLOKitDocument();
            // TaskPriority::HIGHEST -> TaskPriority::REPAINT
            if (mostUrgentPriority >= 0 && mostUrgentPriority <= 4)
            {
                return false;
            }
        }

        if (document->hasCallbacks())
        {
            // Have pending COKit callbacks from core.
            return true;
        }

        // Poll our incoming socket from wsd.
        int ret = poll(std::chrono::microseconds(0), /*justPoll=*/true);
        if (ret)
        {
            return true;
        }

        if (document->hasQueueItems())
        {
            return true;
        }
    }

    return false;
#else
    // FIXME - should return true only if there is any input in any of the Kits
    return true;
#endif
}

namespace
{

/// Called by COKit main-loop
void wakeCallback(void* data)
{
    if (!data)
        return;

    return reinterpret_cast<KitSocketPoll*>(data)->kitWakeup();
}

#if !defined(_WIN32)
// "Where to save?" callback used by every kit variant except CODA-W.
//
// When the engine runs an export flow that would normally pop a file picker
// (.uno:ExportToPDF after the PDF Options dialog, etc.), it asks the embedder
// via this callback for an output URL. CODA-W answers synchronously with a
// Win32 native picker (output_file_dialog_from_core, registered below). Every
// other build hands back a fresh path under a tmp dir, lets the engine write
// there, and then defers picker / delivery to the platform's existing
// KIT_CALLBACK_EXPORT_FILE handler:
//   - browser COOL (!MOBILEAPP): the path is under JAILED_DOCUMENT_ROOT so
//     WSD's downloadId-to-file mapping picks it up; the browser's own save
//     dialog appears when the resulting downloadas: triggers a download.
//   - mobile/desktop apps (MOBILEAPP without _WIN32): the path is under the
//     system tmp dir, accessible to the same process; the platform's
//     exportfile: / iOS UIViewController handler shows its own native picker.
void downloadAsFileSaveDialogCallback(const char* suggestedURI, char* result, size_t resultLen)
{
    if (resultLen == 0)
        return;
    result[0] = '\0';

    if (suggestedURI == nullptr || *suggestedURI == '\0')
        return;

    std::string filename;
    try
    {
        const Poco::URI uri{ std::string(suggestedURI) };
        filename = Poco::Path(uri.getPath()).getFileName();
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("File-save callback got bad URI [" << suggestedURI << "]: " << exc.what());
        return;
    }
    if (filename.empty())
    {
        LOG_ERR("File-save callback URI [" << suggestedURI << "] has empty filename");
        return;
    }

#if MOBILEAPP
    // No chroot, no jail. Use a tmp dir that the embedding app process can
    // read for the deferred picker step.
    std::error_code ec;
    const std::string baseDir = std::filesystem::temp_directory_path(ec).string() + "/cool-export/";
    std::filesystem::create_directories(baseDir, ec);
    if (ec)
    {
        LOG_ERR("File-save callback could not create tmp dir [" << baseDir
                << "]: " << ec.message());
        return;
    }
#else
    // Browser COOL: the kit is chroot'd; this path is jail-doc-root-relative
    // so WSD's GET handler under /cool/.../<downloadId> can read it back via
    // FileUtil::buildLocalPathToJail.
    const std::string baseDir = JAILED_DOCUMENT_ROOT;
#endif

    const auto download = FileUtil::createDownloadJailPath(baseDir, filename);
    const std::string outURI = "file://" + download.absolutePath;

    if (outURI.size() + 1 > resultLen)
    {
        LOG_WRN("File-save callback result buffer too small (" << resultLen
                << " < " << outURI.size() + 1 << ')');
        return;
    }
    std::memcpy(result, outURI.c_str(), outURI.size() + 1);
}
#endif

} // namespace

void KitSocketPoll::kitWakeup() {
#if !defined(IOS) && !defined(QTAPP) && !defined(MACOS) && !defined(_WIN32)
    wakeup();
#else
    std::unique_lock<std::mutex> lock(KitSocketPoll::KSPollsMutex);
    if (KitSocketPoll::KSPolls.empty())
        return;

    std::vector<std::shared_ptr<KitSocketPoll>> v;
    for (const auto &i : KitSocketPoll::KSPolls)
    {
        auto sp = i.lock();
        if (sp)
            v.push_back(sp);
    }
    lock.unlock();
    for (const auto &p : v)
        p->wakeup();
#endif
}

/**
 * Register the "any input", "poll" and "wake up" callbacks in COKit and start the LOKit's main loop.
 *
 * The LOKit main loop will use/call these callbacks inside VCL's Yield(), see SvpSalInstance::ImplYield().
 */
void startMainLoop(const COKit* kit, const std::shared_ptr<kit::Office>& loKit, const std::shared_ptr<KitSocketPoll>& mainKit) {
    if (!COKIT_HAS(kit, runLoop))
    {
        LOG_FTL("Kit is missing Unipoll API");
        std::cout << "Fatal: out of date COKit - no Unipoll API\n";
        Util::forcedExit(EX_SOFTWARE);
    }

    loKit->registerAnyInputCallback(anyInputCallback, mainKit.get());
#if defined(_WIN32)
    loKit->registerFileSaveDialogCallback(output_file_dialog_from_core);
#else
    loKit->registerFileSaveDialogCallback(downloadAsFileSaveDialogCallback);
#endif

    LOG_INF("Kit unipoll loop run");

    loKit->runLoop(pollCallback, wakeCallback, mainKit.get());

    LOG_INF("Kit unipoll loop run terminated.");
}

#if !MOBILEAPP

extern "C"
{
    [[maybe_unused]]
    static void sigChildHandler(int pid)
    {
        // Reap the child; will log failures.
        SigUtil::reapZombieChild(pid, /*sighandler=*/true);
    }
}

void copyCertificateDatabaseToTmp(Poco::Path const& jailPath)
{
    std::string certificatePathString = ConfigUtil::getString("certificates.database_path", "");
    if (!certificatePathString.empty())
    {
        auto fileStat = FileUtil::Stat(certificatePathString);

        if (!fileStat.exists() || !fileStat.isDirectory())
        {
            LOG_WRN("Certificate database wasn't copied into the jail as path '" << certificatePathString << "' doesn't exist");
            return;
        }

        Poco::Path certificatePath(certificatePathString);

        Poco::Path jailedCertDBPath(jailPath, "/tmp/certdb");
        Poco::File(jailedCertDBPath).createDirectories();

        bool copied = false;
        for (const char* filename : { "cert8.db", "cert9.db", "secmod.db", "key3.db", "key4.db" })
        {
            bool result = FileUtil::copy(Poco::Path(certificatePath, filename).toString(),
                                Poco::Path(jailedCertDBPath, filename).toString(), false, false);
            copied |= result;
        }
        if (copied)
        {
            LOG_INF("Certificate database files found in '" << certificatePathString << "' and were copied to the jail");
            ::setenv("LO_CERTIFICATE_DATABASE_PATH", "/tmp/certdb", 1);
        }
        else
        {
            LOG_WRN("No Certificate database files could be found in path '" << certificatePathString << "'");
        }
    }
}

#endif

#if defined(QTAPP) || defined(MACOS) || defined(_WIN32)

// with "unipoll" thread that calls cok_init_2 ends up holding the yield mutex in InitVCL()
// kit::Office:runLoop then spawned in another thread ends up stuck. To prevent that call cok_init_2
// and runLoop in the same thread.
// note: at this point in time, it is unclear (to quwex) if cok_init_2 not being in the "main"
// thread will disrupt other things :-) if that is the case maybe we could also ReleaseYieldMutex()
// manually?
std::future<COKit*> initKitRunLoopThread(const std::shared_ptr<KitSocketPoll>& mainKit)
{
        std::promise<COKit*> promise;
        std::future<COKit*> future = promise.get_future();
        std::thread(
            [p = std::move(promise), mainKit]() mutable
            {
                ProcUtil::setThreadName("lokit_runloop");
                setupKitEnvironment("notebookbar");
                COKit* kit =
#if defined(QTAPP)
                    cok_init_2(LO_PATH "/program", nullptr);
#elif defined(MACOS)
                    cok_init_2((getBundlePath() + "/Contents/Frameworks").c_str(), getAppSupportURL().c_str());
#elif defined(_WIN32)
                    cok_init_2(app_installation_path.c_str(), nullptr);
#endif
                p.set_value(kit);

                std::shared_ptr<kit::Office> loKit = std::make_shared<kit::Office>(kit);

                startMainLoop(kit, loKit, mainKit);

                // Should never return
                std::abort();
            }).detach();
        return future;
}
#endif // QTAPP
void lokit_main(
#if !MOBILEAPP
                const std::string& childRoot,
                const std::string& jailId,
                const std::string& configId,
                const std::string& sysTemplate,
                const std::string& loTemplate,
                bool noCapabilities,
                bool noSeccomp,
                bool useMountNamespaces,
                bool queryVersion,
                bool displayVersion,
                bool sysTemplateIncomplete,
#else
                int docBrokerSocket,
                const std::string& userInterface,
#endif
                std::size_t numericIdentifier
                )
{
#if !MOBILEAPP

    if constexpr (!Util::isKitInProcess())
    {
        // Already set by COOLWSD.cpp
        SigUtil::setFatalSignals("kit startup of " + Util::getCoolVersion() + ' ' +
                                 Util::getCoolVersionHash());
        SigUtil::setUserSignals();
    }

    // Are we the first ever kit ? if so, we havn't tweaked our logging by
    // the time we get here; FIXME: much of this is un-necessary duplication.

    // Reinitialize logging when forked.
    const bool logToFile = std::getenv("COOL_LOGFILE");
    const char* logFilename = std::getenv("COOL_LOGFILENAME");
    const char* logLevel = std::getenv("COOL_LOGLEVEL");
    const char* logDisabledAreas = std::getenv("COOL_LOGDISABLED_AREAS");
    const char* logLevelStartup = std::getenv("COOL_LOGLEVEL_STARTUP");
    const bool logColor = ConfigUtil::getBool("logging.color", true) && isatty(fileno(stderr));
    std::map<std::string, std::string> logProperties;
    if (logToFile && logFilename)
    {
        logProperties["path"] = std::string(logFilename);
    }
    const bool logToFileUICmd = std::getenv("COOL_LOGFILE_UICMD");
    const char* logFilenameUICmd = std::getenv("COOL_LOGFILENAME_UICMD");
    std::map<std::string, std::string> logPropertiesUICmd;
    if (logToFileUICmd && logFilenameUICmd)
    {
        logPropertiesUICmd["path"] = std::string(logFilenameUICmd);
    }

    Util::rng::reseed();

    const std::string LogLevel = logLevel ? logLevel : "trace";
    const std::string LogLevelStartup = logLevelStartup ? logLevelStartup : "trace";

    const bool traceStartup = (std::getenv("COOL_TRACE_STARTUP") != nullptr);
    Log::initialize("kit", traceStartup ? LogLevelStartup : LogLevel, logColor, logToFile,
                    logProperties, logToFileUICmd, logPropertiesUICmd);
    if (traceStartup && LogLevel != LogLevelStartup)
    {
        LOG_INF("Setting log-level to [" << LogLevelStartup << "] and delaying "
                "setting to [" << LogLevel << "] until after Kit initialization.");
    }
    const std::string LogDisabledAreas = logDisabledAreas ? logDisabledAreas : "";

    if (const char* anonymizationSalt = std::getenv("COOL_ANONYMIZATION_SALT"))
    {
        const auto salt = std::stoull(anonymizationSalt);
        const char* highStrengthEnv = std::getenv("COOL_ANONYMIZATION_HIGH_STRENGTH");
        const bool highStrength = highStrengthEnv && std::string(highStrengthEnv) == "1";
        Anonymizer::initialize(true, salt, highStrength);
    }

    LOG_INF("User-data anonymization is " << (Anonymizer::enabled() ? "enabled." : "disabled."));

    const char* enableWebsocketURP = std::getenv("ENABLE_WEBSOCKET_URP");
    EnableWebsocketURP = enableWebsocketURP && std::string(enableWebsocketURP) == "true";

    assert(!childRoot.empty());
    assert(!sysTemplate.empty());
    assert(!loTemplate.empty());

    LOG_INF("Kit process for Jail [" << jailId << "] started.");

    std::string userdir_url;
    std::string instdir_path;
    int ProcSMapsFile = -1;

    // lokit's destroy typically throws from
    // framework/source/services/modulemanager.cxx:198
    // So we insure it lives until std::_Exit is called.
    std::shared_ptr<kit::Office> loKit;
    ChildSession::NoCapsForKit = noCapabilities;
#endif // MOBILEAPP

    // Setup the OSL sandbox
    std::string allowedPaths;

    try
    {
#if !MOBILEAPP
        const Path jailPath = Path::forDirectory(childRoot + '/' + jailId);
        const std::string jailPathStr = jailPath.toString();
        JailUtil::createJailPath(jailPathStr);

        // initialize while we have access to /proc/self/task
        threadCounter.reset(new Util::ThreadCounter());
#ifdef FDCOUNTER_USABLE
        // initialize while we have access to /proc/self/fd
        fdCounter.reset(new Util::FDCounter());
#endif

        bool usingMountNamespace = false;
        std::chrono::milliseconds jailSetupTime(0);

        if (!ChildSession::NoCapsForKit)
        {
            std::chrono::time_point<std::chrono::steady_clock> jailSetupStartTime
                = std::chrono::steady_clock::now();

            userdir_url = "file:///tmp/user";
#ifndef __APPLE__
            instdir_path = '/' + std::string(JailUtil::LO_JAIL_SUBPATH) + "/program";
#else
            instdir_path = '/' + std::string(JailUtil::LO_JAIL_SUBPATH) + "/Contents/Frameworks";
#endif
            allowedPaths += ":r:/" + std::string(JailUtil::LO_JAIL_SUBPATH);

            Poco::Path jailLOInstallation(jailPath, JailUtil::LO_JAIL_SUBPATH);
            jailLOInstallation.makeDirectory();
            const std::string loJailDestPath = jailLOInstallation.toString();

            const std::string tempRoot = Poco::Path(childRoot, "tmp").toString();
            const std::string tmpSubDir = Poco::Path(tempRoot, "cool-" + jailId).toString();
            const std::string jailTmpDir = Poco::Path(jailPath, "tmp").toString();

            const std::string sharedPresets = Poco::Path(childRoot, JailUtil::CHILDROOT_TMP_SHARED_PRESETS_PATH).toString();
            const std::string configIdPresets = Poco::Path(sharedPresets, Uri::encode(configId)).toString();

            const std::string sharedAutotext = Poco::Path(configIdPresets, "autotext").toString();
            const std::string loJailDestAutotextPath = Poco::Path(loJailDestPath, "share/autotext/common").toString();

            const std::string sharedWordbook = Poco::Path(configIdPresets, "wordbook").toString();
            const std::string loJailDestWordbookPath = Poco::Path(loJailDestPath, "share/wordbook").toString();

            const std::string sharedTemplate = Poco::Path(configIdPresets, "template").toString();
            const std::string loJailDestImpressTemplatePath = Poco::Path(loJailDestPath, "share/template/common/presnt").toString();

            const std::string sysTemplateSubDir = Poco::Path(tempRoot, "systemplate-" + jailId).toString();
            const std::string jailEtcDir = Poco::Path(jailPath, "etc").toString();

#if ENABLE_CHILDROOTS
            if (sysTemplateIncomplete && JailUtil::isBindMountingEnabled())
            {
                const std::string sysTemplateEtcDir = Poco::Path(sysTemplate, "etc").toString();
                const std::string sysTemplateSubEtcDir =
                    Poco::Path(sysTemplateSubDir, "etc").toString();
                Poco::File(sysTemplateSubEtcDir).createDirectories();

                FileUtil::copyDirectoryRecursive(sysTemplateEtcDir, sysTemplateSubEtcDir, false);

                if (!JailUtil::SysTemplate::updateDynamicFiles(sysTemplateSubDir))
                {
                    LOG_WRN("Failed to update the dynamic files in ["
                            << sysTemplateSubDir
                            << "]. Will clone systemplate into the "
                               "jails, which is more resource intensive.");
                    JailUtil::disableBindMounting(); // We can't mount from incomplete systemplate.
                }
            }
#endif

            // The bind-mount implementation: inlined here to mirror
            // the fallback link/copy version bellow.
            const auto mountJail = [&]() -> bool {
                // Mount sysTemplate for the jail directory.
                LOG_INF("Mounting " << sysTemplate << " -> " << jailPathStr);
                if (!JailUtil::bind(sysTemplate, jailPathStr)
                    || !JailUtil::remountReadonly(sysTemplate, jailPathStr))
                {
                    LOG_ERR("Failed to mount [" << sysTemplate << "] -> [" << jailPathStr
                                                << "], will link/copy contents.");
                    return false;
                }

                // if we know that the etc dir of sysTemplate is out of date, then
                // ro bind-mount a replacement up-to-date /etc
                if (sysTemplateIncomplete)
                {
                    const std::string sysTemplateSubDirEtc =
                        Poco::Path(sysTemplateSubDir, "etc").toString();
                    LOG_INF("Mounting " << sysTemplateSubDirEtc << " -> " << jailEtcDir);
                    if (!JailUtil::bind(sysTemplateSubDirEtc, jailEtcDir) ||
                        !JailUtil::remountReadonly(sysTemplateSubDirEtc, jailEtcDir))
                    {
                        LOG_ERR("Failed to mount [" << sysTemplateSubDirEtc << "] -> ["
                                                    << jailEtcDir << "], will link/copy contents.");
                        return false;
                    }
                }

                // Mount loTemplate inside it.
                LOG_INF("Mounting " << loTemplate << " -> " << loJailDestPath);
                if (!FileUtil::Stat(loJailDestPath).exists())
                {
                    LOG_DBG("The mount-point [" << loJailDestPath
                                                << "] doesn't exist. Binding will likely fail");
                }

                if (!JailUtil::bind(loTemplate, loJailDestPath)
                    || !JailUtil::remountReadonly(loTemplate, loJailDestPath))
                {
                    LOG_WRN("Failed to mount [" << loTemplate << "] -> [" << loJailDestPath
                                                << "], will link/copy contents");
                    return false;
                }

                if (FileUtil::Stat("/nix/store").exists()) {
                    // Bind-mount /nix/store to the jail as otherwise we will likely get missing fonts/etc.
                    // We won't quit if we fail (e.g. the non-nixos case could work) but unless we're doing something special COOLWSD is unlikely to work without this on nixos
                    const std::string jailNixDir = Poco::Path(jailPath, "nix/store").toString();
                    if (!JailUtil::bind("/nix/store", jailNixDir)
                        || !JailUtil::remountReadonly("/nix/store", jailNixDir))
                    {
                        LOG_WRN("Failed to mount [/nix/store] -> [" << jailNixDir
                                                    << "], ignoring. If you have used nix for dependencies COOLWSD is likely to fail");
                    }
                }

                if (!configId.empty())
                {
                    // mount the shared autotext over the lo shared autotext's 'common' dir
                    if (!JailUtil::bind(sharedAutotext, loJailDestAutotextPath)
                        || !JailUtil::remountReadonly(sharedAutotext, loJailDestAutotextPath))
                    {
                        // TODO: actually do this link on failure
                        LOG_WRN("Failed to mount [" << sharedAutotext << "] -> ["
                                                    << loJailDestAutotextPath
                                                    << "], will link contents");
                        return false;
                    }

                    // mount the shared wordbook over the lo shared wordbook
                    if (!JailUtil::bind(sharedWordbook, loJailDestWordbookPath)
                        || !JailUtil::remountReadonly(sharedWordbook, loJailDestWordbookPath))
                    {
                        // TODO: actually do this link on failure
                        LOG_WRN("Failed to mount [" << sharedWordbook << "] -> [" << loJailDestWordbookPath
                                                    << "], will link contents");
                        return false;

                    }

                    // mount the shared templates over the lo shared templates' 'common' dir
                    if (!JailUtil::bind(sharedTemplate, loJailDestImpressTemplatePath) ||
                        !JailUtil::remountReadonly(sharedTemplate, loJailDestImpressTemplatePath))
                    {
                        LOG_WRN("Failed to mount [" << sharedTemplate << "] -> ["
                                                    << loJailDestImpressTemplatePath
                                                    << "], will link contents");
                        return false;
                    }
                }

                // tmpdir inside the jail for added security.
                Poco::File(tmpSubDir).createDirectories();
                LOG_INF("Mounting random temp dir " << tmpSubDir << " -> " << jailTmpDir);
                if (!JailUtil::bind(tmpSubDir, jailTmpDir))
                {
                    LOG_ERR("Failed to mount [" << tmpSubDir << "] -> [" << jailTmpDir
                                                << "], will link/copy contents.");
                    return false;
                }

                return true;
            };

#ifndef __FreeBSD__
            const uid_t origuid = geteuid();
            const gid_t origgid = getegid();

            // create a namespace and map to root uid/gid
            if (useMountNamespaces)
            {
                LOG_DBG("Move into user namespace as uid 0");
                if (!JailUtil::enterMountingNS(origuid, origgid))
                    LOG_ERR("Linux mount namespace for kit failed: " << strerror(errno));
                else
                    usingMountNamespace = true;
            }
#endif

            // Copy (link) LO installation and other necessary files into it from the template.
            bool bindMount = JailUtil::isBindMountingEnabled();
            if (bindMount)
            {
#if CODE_COVERAGE
                // Code coverage is not supported with bind-mounting.
                LOG_ERR("Mounting is not compatible with code-coverage.");
                assert(!"Mounting is not compatible with code-coverage.");
#endif // CODE_COVERAGE

                if (!mountJail())
                {
                    LOG_INF("Cleaning up jail before linking/copying.");
                    JailUtil::tryRemoveJail(jailPathStr);
                    bindMount = false;
                    JailUtil::disableBindMounting();
                }

            }

#ifndef __FreeBSD__
#if HAVE_LIBCAP
            if (usingMountNamespace)
            {
                // create another namespace, map back to original uid/gid after mount
                cap_t caps = cap_get_proc();
                if (caps != nullptr)
                {
                    char* capText = cap_to_text(caps, nullptr);
                    LOG_TRC("Caps[" << capText << "] before entering nested usernamespace");
                    cap_free(capText);
                    cap_free(caps);
                }
                LOG_DBG("Move into user namespace as uid " << origuid);
                if (!JailUtil::enterUserNS(origuid, origgid))
                    LOG_ERR("Linux user namespace for kit failed: " << strerror(errno));
                caps = cap_get_proc();
                if (caps != nullptr)
                {
                    char* capText = cap_to_text(caps, nullptr);
                    LOG_TRC("Caps[" << capText << "] after entering nested usernamespace");
                    cap_free(capText);
                    cap_free(caps);
                }
            }

            assert(origuid == geteuid());
            assert(origgid == getegid());
#endif
#endif

            if (!bindMount)
            {
                LOG_INF("Mounting is disabled, will link/copy " << sysTemplate << " -> "
                                                                << jailPathStr);

                // Make sure we have the jail directory.
                JailUtil::createJailPath(jailPathStr);

                // Create a file to mark this a copied jail.
                JailUtil::markJailCopied(jailPathStr);

                const std::string linkablePath = childRoot + "/linkable";

                linkOrCopy(sysTemplate, jailPath, linkablePath, LinkOrCopyType::All);

                linkOrCopy(loTemplate, loJailDestPath, linkablePath, LinkOrCopyType::LO);

                if (!configId.empty())
                    linkOrCopy(sharedTemplate, loJailDestImpressTemplatePath + "/", linkablePath,
                               LinkOrCopyType::All);

#if CODE_COVERAGE
                // Link the .gcda files.
                linkGCDAFiles(jailPathStr);
#endif

#if ENABLE_CHILDROOTS
                // Update the dynamic files inside the jail.
                if (!JailUtil::SysTemplate::updateDynamicFiles(jailPathStr))
                {
                    LOG_ERR(
                        "Failed to update the dynamic files in the jail ["
                        << jailPathStr
                        << "]. If the systemplate directory is owned by a superuser or is "
                           "read-only, running the installation scripts with the owner's account "
                           "should update these files. Some functionality may be missing.");
                }
#endif

                if (usingMountNamespace)
                {
                    Poco::File(tempRoot).createDirectories();
                    if (symlink(jailTmpDir.c_str(), tmpSubDir.c_str()) == -1)
                    {
                        LOG_SFL("Failed to create symlink [" << tmpSubDir << " -> " << jailTmpDir << "] " << strerror(errno));
                        Util::forcedExit(EX_SOFTWARE);
                    }
                }
            }

            // Setup /tmp and set TMPDIR.
            ::setenv("TMPDIR", "/tmp", 1);
            allowedPaths += ":w:/tmp";

            copyCertificateDatabaseToTmp(jailPath);

            // HOME must be writable, so create it in /tmp.
            constexpr const char* HomePathInJail = "/tmp/home";
            Poco::File(Poco::Path(jailPath, HomePathInJail)).createDirectories();
            ::setenv("HOME", HomePathInJail, 1);

            jailSetupTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - jailSetupStartTime);
            LOG_DBG("Initialized jail files in " << jailSetupTime);

            // The bug is that rewinding and rereading /proc/self/smaps_rollup doubles the previous
            // values, so it only affects the case where we reuse the fd from opening smaps_rollup
            const bool brokenSmapsRollup = (std::getenv("COOL_DISABLE_SMAPS_ROLLUP") != nullptr);
            ProcSMapsFile = !brokenSmapsRollup ? open("/proc/self/smaps_rollup", O_RDONLY) : -1;
            if (ProcSMapsFile < 0)
            {
                if (!brokenSmapsRollup)
                    LOG_WRN("Failed to open /proc/self/smaps_rollup. Memory stats will be slower");
                ProcSMapsFile = open("/proc/self/smaps", O_RDONLY);
                if (ProcSMapsFile < 0)
                    LOG_SYS("Failed to open /proc/self/smaps. Memory stats will be missing.");
            }

            LOG_INF("chroot(\"" << jailPathStr << "\")");
            if (chroot(jailPathStr.c_str()) == -1)
            {
                LOG_SFL("chroot(\"" << jailPathStr << "\") failed");
                Util::forcedExit(EX_SOFTWARE);
            }

            if (chdir("/") == -1)
            {
                LOG_SFL("chdir(\"/\") in jail failed");
                Util::forcedExit(EX_SOFTWARE);
            }

#if HAVE_LIBCAP
            if (usingMountNamespace)
            {
                // We have a full set of capabilities in the namespace so drop
                // them all
                if (dropAllCapabilities() == -1)
                    LOG_ERR("Failed to drop all capabilities");
            }
            else
            {
                dropCapability(CAP_SYS_CHROOT);
                dropCapability(CAP_FOWNER);
                dropCapability(CAP_CHOWN);
            }

            char *capText = cap_to_text(cap_get_proc(), nullptr);
            LOG_DBG("Initialized jail nodes, dropped caps. Final caps are: " << capText);
            cap_free(capText);
#endif
        }
        else // noCapabilities set
        {
            LOG_WRN("Security warning: running without chroot jails is insecure.");
            LOG_INF("Using template ["
                    << loTemplate << "] as install subpath directly, without chroot jail setup.");
            userdir_url = "file://" + jailPathStr + "tmp/user";
#ifndef __APPLE__
            instdir_path = '/' + loTemplate + "/program";
#else
            instdir_path = '/' + loTemplate + "/Contents/Frameworks";
#endif
            allowedPaths += ":r:" + loTemplate;
            JailRoot = jailPathStr;

            std::string tmpPath = jailPathStr + "tmp";
            ::setenv("TMPDIR", tmpPath.c_str(), 1);
            allowedPaths += ":w:" + tmpPath;
            LOG_DBG("Using tmpdir [" << tmpPath << "]");

            // used by LO Migration::migrateSettingsIfNecessary() in startup code as config dir
            ::setenv("XDG_CONFIG_HOME", (tmpPath + "/.config").c_str(), 1);
            ::setenv("HOME", tmpPath.c_str(), 1);
            // overwrite coolkitconfig.xcu setting to fit into allowed paths
            ::setenv("KIT_WORKDIR", ("file://" + tmpPath).c_str(), 1);

            // Setup the OSL sandbox
            allowedPaths += ":r:" + pathFromFileURL(userdir_url);
            ::setenv("SAL_ALLOWED_PATHS", allowedPaths.c_str(), 1);

#if ENABLE_DEBUG
            ::setenv("SAL_ABORT_ON_FORBIDDEN", "1", 1);
#endif
        }

        LOG_DBG("Initializing COKit with instdir [" << instdir_path << "] and userdir ["
                                                  << userdir_url << "].");

        UserDirPath = pathFromFileURL(userdir_url);
        InstDirPath = instdir_path;

        COKit* kit = nullptr;
        {
            const char *instdir = instdir_path.c_str();
            const char *userdir = userdir_url.c_str();

            if (!initFunction)
                initFunction = cok_init_2;

            if constexpr (!Util::isKitInProcess())
                kit = UnitKit::get().cok_init(instdir, userdir, initFunction);
            if (!kit)
                kit = initFunction(instdir, userdir);

            loKit = std::make_shared<kit::Office>(kit);
            if (!loKit)
            {
                LOG_FTL("COKit initialization failed. Exiting.");
                Util::forcedExit(EX_SOFTWARE);
            }
        }

        bool hasSeccomp = false;
        // Lock down the syscalls that can be used
        if (!Seccomp::lockdown(Seccomp::Type::KIT))
        {
            if (!noSeccomp)
            {
                LOG_FTL("COKit seccomp security lockdown failed. Exiting.");
                Util::forcedExit(EX_SOFTWARE);
            }

            LOG_ERR("COKit seccomp security lockdown failed, but configured to continue. "
                    "You are running in a significantly less secure mode.");
        }
        else
        {
#if DISABLE_SECCOMP == 0
            hasSeccomp = true;
#else
            hasSeccomp = false;
#endif
        }

        rlimit rlim = { 0, 0 };
        if (getrlimit(RLIMIT_AS, &rlim) == 0)
            LOG_INF("RLIMIT_AS is " << Util::getHumanizedBytes(rlim.rlim_max) << " (" << rlim.rlim_max << " bytes)");
        else
            LOG_SYS("Failed to get RLIMIT_AS");

        if (getrlimit(RLIMIT_STACK, &rlim) == 0)
            LOG_INF("RLIMIT_STACK is " << Util::getHumanizedBytes(rlim.rlim_max) << " (" << rlim.rlim_max << " bytes)");
        else
            LOG_SYS("Failed to get RLIMIT_STACK");

        if (getrlimit(RLIMIT_FSIZE, &rlim) == 0)
            LOG_INF("RLIMIT_FSIZE is " << Util::getHumanizedBytes(rlim.rlim_max) << " (" << rlim.rlim_max << " bytes)");
        else
            LOG_SYS("Failed to get RLIMIT_FSIZE");

        if (getrlimit(RLIMIT_NOFILE, &rlim) == 0)
            LOG_INF("RLIMIT_NOFILE is " << rlim.rlim_max << " files.");
        else
            LOG_SYS("Failed to get RLIMIT_NOFILE");

        LOG_INF("Kit process for Jail [" << jailId << "] is ready.");

        std::string pathAndQuery(NEW_CHILD_URI);
        pathAndQuery.append("?jailid=");
        pathAndQuery.append(jailId);
        if (!configId.empty())
        {
            pathAndQuery.append("&configid=");
            pathAndQuery.append(configId);
        }
        if (queryVersion)
        {
            LOKitHelper::ScopedString versionInfo(loKit->getVersionInfo());
            std::string versionString(versionInfo.get());
            if (displayVersion)
                std::cout << "office version details: " << versionString << std::endl;

            SigUtil::setVersionInfo(versionString);

            LOG_INF("Kit core version is " << versionString);

            // Extend the list on new releases
            static const char *denyVersions[] = {
                "\"22.05\"", "\"23.05\""
            };
            for (auto const &deny: denyVersions)
            {
                if (Util::findSubArray(versionString.c_str(), versionString.length(),
                                       deny, strlen(deny)) >= 0)
                {
                    LOG_FTL("Mis-matching, obsolete core version, "
                            "please update your packages: " << versionString);
                    Util::forcedExit(EX_SOFTWARE);
                }
            }

            // Add some parameters we want to pass to the client. Could not figure out how to get
            // the configuration parameters from COOLWSD.cpp's initialize() or coolwsd.xml here, so
            // oh well, just have the value hardcoded in KitHelper.hpp. It isn't really useful to
            // "tune" it at end-user installations anyway, I think.
            auto versionJSON = Poco::JSON::Parser().parse(versionString).extract<Poco::JSON::Object::Ptr>();
            versionJSON->set("tunnelled_dialog_image_cache_size", std::to_string(LOKitHelper::tunnelledDialogImageCacheSize));
            std::stringstream ss;
            versionJSON->stringify(ss);
            versionString = ss.str();

            std::string encodedVersion;
            Poco::URI::encode(versionString, "?#/", encodedVersion);
            pathAndQuery.append("&version=");
            pathAndQuery.append(encodedVersion);
        }

        // Admin settings bits:
        // Are we using seccomp ?
        pathAndQuery.append(std::string("&adms_seccomp=") +
                            (hasSeccomp ? "ok" : "none"));
        // Are we bind mounting ?
        pathAndQuery.append(std::string("&adms_bindmounted=") +
                            (!JailUtil::isBindMountingConfigured()
                                 ? "not_recommended"
                                 : (JailUtil::isBindMountingEnabled() ? "ok" : "slow")));
        // Are we using a container - either chroot or namespace ?
        pathAndQuery.append(std::string("&adms_contained=") +
                            (ChildSession::NoCapsForKit ? "uncontained" : "ok"));
        // How slow was the jail setup ?
        pathAndQuery.append(std::string("&adms_info_setup_ms=") +
                            std::to_string(jailSetupTime.count()));
        // Are we using namespaces (or CAP_SYS_CHROOT etc.)
        pathAndQuery.append(std::string("&adms_info_namespaces=") +
                            (useMountNamespaces ? "true" : "false"));

#endif // !MOBILEAPP

        auto mainKit = KitSocketPoll::create();
        mainKit->runOnClientThread(); // We will do the polling on this thread.

#if MOBILEAPP && !defined(IOS) && !defined(QTAPP) && !defined(MACOS) && !defined(_WIN32)
        // For iOS we call it in -[AppDelegate application: didFinishLaunchingWithOptions:]
        // For QTAPP/MACOS/_WIN32 it is called in initKitRunLoopThread()
        setupKitEnvironment(userInterface);
#endif

#if MOBILEAPP
#if (defined(__linux__) && !defined(__ANDROID__) && !defined(QTAPP)) || defined(__FreeBSD__)
        Poco::URI userInstallationURI("file", LO_PATH);
        COKit *kit = cok_init_2(LO_PATH "/program", userInstallationURI.toString().c_str());
#elif defined(IOS) // In the iOS app we call cok_init_2() just once, when the app starts
        static COKit *kit = lo_kit;
#elif defined(QTAPP) || defined(MACOS) || defined(_WIN32)
        // For macOS, this is the MOBILEAPP case
        static COKit* kit = initKitRunLoopThread(mainKit).get();
#else
        // FIXME: I wonder for which platform this is supposed to be? Android?
        static COKit *kit = cok_init_2(nullptr, nullptr);
#endif

        assert(kit);

        static std::shared_ptr<kit::Office> loKit = std::make_shared<kit::Office>(kit);
        assert(loKit);

        COOLWSD::LOKitVersion = loKit->getVersionInfo();

        // Dummies
        const std::string jailId = "jailid";

#endif // MOBILEAPP

        std::shared_ptr<KitWebSocketHandler> websocketHandler =
            std::make_shared<KitWebSocketHandler>("child_ws", loKit, jailId, mainKit, numericIdentifier);

#if !MOBILEAPP

        std::vector<int> shareFDs;
        if (ProcSMapsFile >= 0)
            shareFDs.push_back(ProcSMapsFile);

        if (isURPEnabled())
        {
            if (Syscall::pipe2(URPtoLoFDs, O_CLOEXEC) != 0 || Syscall::pipe2(URPfromLoFDs, O_CLOEXEC | O_NONBLOCK) != 0)
                LOG_ERR("Failed to create urp pipe " << strerror(errno));
            else
            {
                shareFDs.push_back(URPtoLoFDs[1]);
                shareFDs.push_back(URPfromLoFDs[0]);
            }
        }

        if (!mainKit->insertNewUnixSocket(MasterLocation, pathAndQuery, websocketHandler,
                                          &shareFDs))
        {
            LOG_SFL("Failed to connect to WSD. Will exit.");
            Util::forcedExit(EX_SOFTWARE);
        }
#else
        bool fatalError =
            !mainKit->insertNewFakeSocket(docBrokerSocket, websocketHandler);
        if (fatalError)
            LOG_SYS("Fatal error connecting to socket #" << docBrokerSocket);
#endif

        LOG_INF("New kit client websocket inserted.");

#if !MOBILEAPP

        // Since we don't track the bg-save process,
        // for example to prevent multiple parallel saves,
        // we could, in principle, ignore SIGCHLD and avoid
        // the problem of zombies and reaping. Unfortunately,
        // ignoring SIGCHLD is not portable, according to
        // man 2 sigaction. So we simply waitpid(2) on SIGCHLD.
        SigUtil::setSigChildHandler(sigChildHandler);

        if (traceStartup && LogLevel != LogLevelStartup)
        {
            LOG_INF("Kit initialization complete: setting log-level to [" << LogLevel << "] as configured.");
            Log::setLevel(LogLevel);
        }
        Log::setDisabledAreas(LogDisabledAreas);
#endif

#if !defined(IOS) && !defined(QTAPP) && !defined(MACOS) && !defined(_WIN32)
        startMainLoop(kit, loKit, mainKit);

        // Trap the signal handler, if invoked,
        // to prevent exiting.
        LOG_INF("Kit process for Jail [" << jailId << "] finished.");

        // Let forkit handle the jail cleanup.

#else // IOS or QTAPP or MACOS or _WIN32
        auto const termination = mainKit->termination;
#if defined(QTAPP) || defined(MACOS) || defined(_WIN32)
        // Release the mainKit KitSocketPoll instance early here, so that its destructor will
        // reliably be called on the expected "lokit_runloop" owner thread (started by
        // initKitRunLoopThread), avoiding a race between this thread releasing its shared reference
        // when mainKit goes out of scope and the "lokit_runloop" thread releasing its shared
        // reference when it releases the KitSocketPoll instance at the end of
        // KitWebSocketHandler::onDisconnect (in kit/KitWebSocket.cpp):
        mainKit.reset();
#endif
        if (!fatalError)
        {
            std::unique_lock<std::mutex> lock(termination->mutex);
            termination->cv.wait(lock,[&]{ return termination->flag; } );
        }
#endif // !defined(IOS) && !defined(QTAPP) && !defined(MACOS) && !defined(_WIN32)
    }
    catch (const Exception& exc)
    {
        LOG_ERR("Poco Exception: " << exc.displayText() <<
                (exc.nested() ? " (" + exc.nested()->displayText() + ')' : ""));
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Exception: " << exc.what());
    }

#if !MOBILEAPP

    LOG_INF("Kit process for Jail [" << jailId << "] finished.");
    flushTraceEventRecordings();
    if constexpr (!Util::isKitInProcess())
        Util::forcedExit(EX_OK);

#endif
}

#ifdef IOS

// In the iOS app we can have several documents open in the app process at the same time, thus
// several lokit_main() functions running at the same time. We want just one LO main loop, though,
// so we start it separately in its own thread.
void runKitLoopInAThread()
{
    std::thread([&]
                {
                    ProcUtil::setThreadName("lokit_runloop");

                    std::shared_ptr<kit::Office> loKit = std::make_shared<kit::Office>(lo_kit);
                    int dummy;
                    loKit->runLoop(pollCallback, wakeCallback, &dummy);

                    // Should never return
                    assert(false);

#if defined(IOS)
                    NSLog(@"loKit->runLoop() unexpectedly returned");
#endif

                    std::abort();
                }).detach();
}

#endif // IOS

#endif // !BUILDING_TESTS

void consistencyCheckJail()
{
    static bool warned = false;
    if (!warned)
    {
        bool failedTmp, failedLo, failedUser;
        FileUtil::Stat tmp("/tmp");
        if ((failedTmp = (!tmp.good() || !tmp.isDirectory())))
            LOG_ERR("Fatal system error: Kit jail is missing its /tmp directory");

#ifndef __APPLE__
        FileUtil::Stat lo(InstDirPath + "/unorc");
#else
        FileUtil::Stat lo(InstDirPath + "/../Resources/ure/etc/unorc");
#endif
        if ((failedLo = (!lo.good() || !lo.isFile())))
            LOG_ERR("Fatal system error: Kit jail is missing its COKit directory at '" << InstDirPath << "'");

        FileUtil::Stat user(UserDirPath);
        if ((failedUser = (!user.good() || !user.isDirectory())))
            LOG_ERR("Fatal system error: Kit jail is missing its user directory at '" << UserDirPath << "'");

        if (failedTmp || failedLo || failedUser)
        {
            LOG_ERR("A fatal system error indicates that, outside the control of COOL "
                    "major structural changes have occurred in our filesystem. These are "
                    "potentially indicative of an operator damaging the system, and will "
                    "inevitably cause document data-loss and/or malfunction.");
            warned = true;
            SigUtil::addActivity("Fatal, inconsistent jail detected.");
            assert(!"Fatal system error with jail setup.");
        }
        else
            LOG_TRC("Passed system consistency check");
    }
}

/// Fetch the latest monotonically incrementing wire-id
TileWireId getCurrentWireId(bool increment)
{
    return RenderTiles::getCurrentWireId(increment);
}

std::string anonymizeUrl(const std::string& url)
{
#ifndef BUILDING_TESTS
    return Anonymizer::anonymizeUrl(url);
#else
    return url;
#endif
}

#if !MOBILEAPP

static int receiveURPData(void* context, const signed char* buffer, size_t bytesToWrite)
{
    const signed char *ptr = buffer;
    while (bytesToWrite > 0)
    {
        ssize_t bytes = ::write(reinterpret_cast<intptr_t>(context), ptr, bytesToWrite);
        if (bytes <= 0)
            break;
        bytesToWrite -= bytes;
        ptr += bytes;
    }
    return ptr - buffer;
}

static size_t sendURPData(void* context, signed char* buffer, size_t bytesToRead)
{
    signed char *ptr = buffer;
    while (bytesToRead > 0)
    {
        ssize_t bytes = ::read(reinterpret_cast<intptr_t>(context), ptr, bytesToRead);
        if (bytes <= 0)
            break;
        bytesToRead -= bytes;
        ptr += bytes;
    }
    return ptr - buffer;
}

static int receiveURPFromLO(void* context, const signed char* buffer, int bytesToWrite)
{
    assert(bytesToWrite >= 0 && "cannot be negative");
    return receiveURPData(context, buffer, bytesToWrite);
}

static int sendURPToLO(void* context, signed char* buffer, int bytesToRead)
{
    assert(bytesToRead >= 0 && "cannot be negative");
    return sendURPData(context, buffer, bytesToRead);
}

bool startURP(const std::shared_ptr<kit::Office>& LOKit, void** ppURPContext)
{
    if (!isURPEnabled())
    {
        LOG_ERR("URP/WS: Attempted to start a URP session but URP is disabled");
        return false;
    }
    if (URPStartCount > 0)
    {
        LOG_WRN("URP/WS: Not starting another URP session as one has already been opened for this "
                "kit instance");
        return false;
    }

    *ppURPContext = LOKit->startURP(reinterpret_cast<void*>(URPfromLoFDs[1]),
                                    reinterpret_cast<void*>(URPtoLoFDs[0]),
                                    receiveURPFromLO, sendURPToLO);

    if (!*ppURPContext)
    {
        LOG_ERR("URP/WS: tried to start a URP session but core did not let us");
        return false;
    }

    URPStartCount++;
    return true;
}

/// Initializes COKit for cross-fork re-use.
bool globalPreinit(const std::string &loTemplate)
{
    std::string loadedLibrary;
    // we deliberately don't dlclose handle on success, make it
    // static so static analysis doesn't see this as a leak
    static void *handle;
#ifndef __APPLE__
    std::string libMerged = loTemplate + "/program/libmergedlo.so";
#else
    std::string libMerged = loTemplate + "/Contents/Frameworks/libmergedlo.dylib";
#endif
    if (File(libMerged).exists())
    {
        LOG_TRC("dlopen(" << libMerged << ", RTLD_GLOBAL|RTLD_NOW)");
        handle = dlopen(libMerged.c_str(), RTLD_GLOBAL|RTLD_NOW);
        if (!handle)
        {
            LOG_FTL("Failed to load " << libMerged << ": " << dlerror());
            return false;
        }
        loadedLibrary = std::move(libMerged);
    }
    else
    {
#ifndef __APPLE__
        std::string libSofficeapp = loTemplate + "/program/libsofficeapp.so";
#else
        std::string libSofficeapp = loTemplate + "/Contents/Frameworks/libsofficeapp.dylib";
#endif
        if (File(libSofficeapp).exists())
        {
            LOG_TRC("dlopen(" << libSofficeapp << ", RTLD_GLOBAL|RTLD_NOW)");
            handle = dlopen(libSofficeapp.c_str(), RTLD_GLOBAL|RTLD_NOW);
            if (!handle)
            {
                LOG_FTL("Failed to load " << libSofficeapp << ": " << dlerror());
                return false;
            }
            loadedLibrary = std::move(libSofficeapp);
        }
        else
        {
            LOG_FTL("Neither " << libSofficeapp << " or " << libMerged << " exist.");
            return false;
        }
    }

    CokHookPreInit2* preInit = reinterpret_cast<CokHookPreInit2 *>(dlsym(handle, "cok_preinit_2"));
    if (!preInit)
    {
        LOG_FTL("No cok_preinit_2 symbol in " << loadedLibrary << ": " << dlerror());
        dlclose(handle);
        return false;
    }

    initFunction = reinterpret_cast<CokHookFunction2 *>(dlsym(handle, "cokit_hook_2"));
    if (!initFunction)
    {
        LOG_FTL("No cokit_hook_2 symbol in " << loadedLibrary << ": " << dlerror());
    }

    // Disable problematic components that may be present from a
    // desktop or developer's install if env. var not set.
    ::setenv("UNODISABLELIBRARY",
             "abp avmediagst avmediavlc cmdmail losessioninstall OGLTrans PresenterScreen "
             "syssh ucpftp1 ucpgio1 ucpimage updatecheckui updatefeed updchk"
             // Database
             "dbaxml dbmm dbp dbu deployment firebird_sdbc mork "
             "mysql mysqlc odbc postgresql-sdbc postgresql-sdbc-impl sdbc2 sdbt"
             // Java
             "javaloader javavm jdbc rpt rptui rptxml ",
             0 /* no overwrite */);

#ifndef __APPLE__
    const std::string lokProgramDir = loTemplate + "/program";
#else
    const std::string lokProgramDir = loTemplate + "/Contents/Frameworks";
#endif

    LOG_TRC("Invoking cok_preinit_2(" << lokProgramDir << ", \"file:///tmp/user\")");
    const auto start = std::chrono::steady_clock::now();
    if (preInit(lokProgramDir.c_str(), "file:///tmp/user", &loKitPtr) != 0)
    {
        LOG_FTL("cok_preinit() in " << loadedLibrary << " failed");
        dlclose(handle);
        return false;
    }

    LOG_DBG("After cok_preinit_2: loKitPtr=" << loKitPtr);

    LOG_TRC("Finished cok_preinit(" << lokProgramDir << ", \"file:///tmp/user\") in "
                                    << std::chrono::duration_cast<std::chrono::milliseconds>(
                                           std::chrono::steady_clock::now() - start));
    return true;
}

/// Anonymize usernames.
std::string anonymizeUsername(const std::string& username)
{
#ifndef BUILDING_TESTS
    return Anonymizer::anonymize(username);
#else
    return username;
#endif
}

#endif // !MOBILEAPP

void dump_kit_state()
{
    std::ostringstream oss(Util::makeDumpStateStream());
    oss << "Start Kit " << ProcUtil::getProcessId() << " Dump State:\n";

    SigUtil::signalLogActivity();

    KitSocketPoll::dumpGlobalState(oss);

    oss << "\nMalloc info [" << ProcUtil::getProcessId() << "]: \n\t"
        << Util::replace(Util::getMallocInfo(), "\n", "\n\t") << '\n';
    oss << "\nEnd Kit " << ProcUtil::getProcessId() << " Dump State.\n";

    const std::string msg = oss.str();
    fprintf(stderr, "%s", msg.c_str()); // Log in the journal.
    LOG_WRN(msg);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
