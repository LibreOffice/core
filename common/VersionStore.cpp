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

#include <config.h>

#include <common/VersionStore.hpp>

#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/SpookyV2.h>
#include <common/Util.hpp>

#include <Poco/JSON/Object.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include <sys/stat.h>
#include <sys/types.h>

namespace versioning
{

namespace
{

constexpr const char* MetaFileName = "meta.json";

/// Files in a document directory that neither are meta.json nor parse as
/// version filenames are swept at startup once older than this; the
/// grace period protects a copy in flight in another app instance.
constexpr std::chrono::seconds OrphanGraceAge{ 24 * 3600 };

struct DocEntry
{
    std::string docKey; ///< Canonical document path; empty if meta.json was lost.
    std::string displayName; ///< The document filename.
    /// Versions sorted oldest-first.
    std::vector<VersionInfo> versions;

    std::uint64_t size() const
    {
        std::uint64_t total = 0;
        for (const VersionInfo& version : versions)
            total += version.size;
        return total;
    }
};

std::mutex StoreMutex;
bool Initialized = false;
Limits StoreLimits;
std::string RootPath;
std::string ExportDir;
std::vector<std::string> ExcludedPrefixes;
/// Document-directory name (key hash) to entry.
std::map<std::string, DocEntry> Index;
std::uint64_t TotalSizeBytes = 0;
/// Document paths with a snapshot in flight.
std::unordered_set<std::string> InFlight;
/// Document paths asked for again while in flight, with the merge-window bypass to rerun with.
std::unordered_map<std::string, bool> Pending;
std::unordered_set<std::string> TooLargeWarned;

std::function<void()> CopyHookForTest;

std::string fileNameOf(const std::string& path)
{
    const std::size_t pos = path.find_last_of("/\\");
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

std::string hashDirName(const std::string& docKey)
{
    const std::uint64_t hash = SpookyHash::Hash64(docKey.data(), docKey.size(), /*seed=*/0);
    char buffer[17];
    std::snprintf(buffer, sizeof(buffer), "%016llx", static_cast<unsigned long long>(hash));
    return buffer;
}

bool isExcluded(const std::string& docPath, const std::string& docKey)
{
    for (const std::string& prefix : ExcludedPrefixes)
    {
        if (prefix.empty())
            continue;
        if (docPath.compare(0, prefix.size(), prefix) == 0 ||
            docKey.compare(0, prefix.size(), prefix) == 0)
            return true;
    }
    return false;
}

time_t timegmPortable(std::tm& tm)
{
#ifdef _WIN32
    return _mkgmtime(&tm);
#else
    return timegm(&tm);
#endif
}

/// A forced exit mid-write leaves only a temporary (swept at the next
/// startup).
bool writeStringAtomic(const std::string& path, const std::string& content)
{
    const std::string tempPath = path + Util::rng::getFilename(12);
    std::ofstream stream;
    FileUtil::openFileToOFStream(tempPath, stream);
    if (!stream.is_open() || stream.bad())
        return false;
    stream << content;
    stream.close();
    if (stream.fail() || ::rename(tempPath.c_str(), path.c_str()) != 0)
    {
        FileUtil::removeFile(tempPath);
        return false;
    }
    return true;
}

std::string docDirPath(const std::string& dirName) { return RootPath + '/' + dirName; }

std::string versionPath(const std::string& dirName, const std::string& fileName)
{
    return RootPath + '/' + dirName + '/' + fileName;
}

void writeMeta(const std::string& dirName, const DocEntry& entry)
{
    Poco::JSON::Object::Ptr object = new Poco::JSON::Object();
    object->set("key", entry.docKey);
    object->set("displayName", entry.displayName);
    if (!writeStringAtomic(versionPath(dirName, MetaFileName), JsonUtil::jsonToString(object)))
        LOG_WRN("VersionStore: failed to write meta for [" << dirName << ']');
}

void loadMeta(const std::string& dirName, DocEntry& entry)
{
    std::string text;
    if (FileUtil::readFile(versionPath(dirName, MetaFileName), text) <= 0)
        return;

    Poco::JSON::Object::Ptr object;
    if (!JsonUtil::parseJSON(text, object))
        return;

    entry.docKey = object->optValue<std::string>("key", std::string());
    entry.displayName = object->optValue<std::string>("displayName", std::string());
}

void sortVersions(DocEntry& entry)
{
    std::sort(entry.versions.begin(), entry.versions.end(),
              [](const VersionInfo& lhs, const VersionInfo& rhs)
              {
                  if (lhs.timestamp != rhs.timestamp)
                      return lhs.timestamp < rhs.timestamp;
                  // Same second: the shorter name came first ("x.odt"
                  // before "x-1.odt", "x-2.odt" before "x-10.odt").
                  if (lhs.fileName.size() != rhs.fileName.size())
                      return lhs.fileName.size() < rhs.fileName.size();
                  return lhs.fileName < rhs.fileName;
              });
}

/// Delete version files of @entry per @indicesNewestFirst (indices into
/// the newest-first view of entry.versions). Caller holds StoreMutex.
void deleteVersions(const std::string& dirName, DocEntry& entry,
                    const std::vector<std::size_t>& indicesNewestFirst)
{
    const std::size_t count = entry.versions.size();
    std::vector<bool> remove(count, false);
    for (const std::size_t newestFirstIndex : indicesNewestFirst)
    {
        if (newestFirstIndex < count)
            remove[count - 1 - newestFirstIndex] = true;
    }

    std::vector<VersionInfo> kept;
    kept.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        if (!remove[i])
        {
            kept.push_back(std::move(entry.versions[i]));
            continue;
        }

        LOG_DBG("VersionStore: pruning [" << entry.versions[i].fileName << "] of ["
                                          << entry.displayName << ']');
        FileUtil::removeFile(versionPath(dirName, entry.versions[i].fileName));
        TotalSizeBytes -= std::min(TotalSizeBytes, entry.versions[i].size);
    }
    entry.versions = std::move(kept);
}

/// Apply per-document retention. Caller holds StoreMutex.
void thinDoc(const std::string& dirName, DocEntry& entry,
             std::chrono::system_clock::time_point now)
{
    std::vector<VersionInfo> newestFirst(entry.versions.rbegin(), entry.versions.rend());
    deleteVersions(dirName, entry,
                   policy::thin(newestFirst, now, StoreLimits.maxVersionsPerDoc));
}

/// Takes @dirName by value: callers may pass the map key of the entry
/// being erased.
void removeDocDir(std::string dirName)
{
    const auto it = Index.find(dirName);
    if (it != Index.end())
    {
        TotalSizeBytes -= std::min(TotalSizeBytes, it->second.size());
        Index.erase(it);
    }
    FileUtil::removeFile(docDirPath(dirName), /*recursive=*/true);
}

/// Enforce the global size cap, leaving @headroomBytes to spare. Caller
/// holds StoreMutex.
void makeSpace(std::uint64_t headroomBytes)
{
    const std::uint64_t target =
        StoreLimits.maxStoreSizeBytes > headroomBytes
            ? StoreLimits.maxStoreSizeBytes - headroomBytes
            : 0;

    while (TotalSizeBytes > target)
    {
        std::map<std::string, DocEntry>::iterator victim = Index.end();
        for (auto it = Index.begin(); it != Index.end(); ++it)
        {
            if (it->second.versions.size() < 2)
                continue;
            if (victim == Index.end() ||
                it->second.versions.size() > victim->second.versions.size() ||
                (it->second.versions.size() == victim->second.versions.size() &&
                 it->second.versions.front().timestamp <
                     victim->second.versions.front().timestamp))
                victim = it;
        }

        if (victim == Index.end())
            break; // Every document is down to a single version.

        deleteVersions(victim->first, victim->second,
                       { victim->second.versions.size() - 1 }); // The oldest.
    }

    while (TotalSizeBytes > target && !Index.empty())
    {
        std::map<std::string, DocEntry>::iterator victim = Index.begin();
        for (auto it = Index.begin(); it != Index.end(); ++it)
        {
            const auto newestOf = [](const DocEntry& entry)
            { return entry.versions.empty() ? std::chrono::system_clock::time_point::min()
                                            : entry.versions.back().timestamp; };
            if (newestOf(it->second) < newestOf(victim->second))
                victim = it;
        }

        LOG_INF("VersionStore: evicting all versions of [" << victim->second.displayName
                                                           << "] to enforce the store cap");
        removeDocDir(victim->first);
    }
}

/// Retention plus the global cap. Caller holds StoreMutex.
void pruneLocked(std::chrono::system_clock::time_point now)
{
    for (auto it = Index.begin(); it != Index.end();)
    {
        thinDoc(it->first, it->second, now);
        if (it->second.versions.empty())
        {
            const std::string dirName = it->first;
            ++it;
            removeDocDir(dirName);
        }
        else
            ++it;
    }

    makeSpace(0);
}

/// Rebuild the index from disk, sweeping orphan temporaries. Caller
/// holds StoreMutex.
void scanStore(std::chrono::system_clock::time_point now)
{
    Index.clear();
    TotalSizeBytes = 0;

    if (!FileUtil::Stat(RootPath).exists())
        return;

    for (const std::string& dirName : FileUtil::getDirEntries(RootPath))
    {
        const std::string dirPath = docDirPath(dirName);
        const FileUtil::Stat dirStat(dirPath);
        if (!dirStat.isDirectory())
        {
            // Nothing legitimate lives directly in the root.
            LOG_WRN("VersionStore: removing stray root entry [" << dirName << ']');
            FileUtil::removeFile(dirPath);
            continue;
        }

        DocEntry entry;
        loadMeta(dirName, entry);

        for (const std::string& fileName : FileUtil::getDirEntries(dirPath))
        {
            if (fileName == MetaFileName)
                continue;

            const std::string filePath = versionPath(dirName, fileName);
            VersionInfo version;
            version.fileName = fileName;
            const FileUtil::Stat fileStat(filePath);
            if (!fileStat.isFile() || !parseVersionFilename(fileName, version.timestamp))
            {
                if (fileStat.modifiedTimepoint() + OrphanGraceAge < now)
                {
                    LOG_WRN("VersionStore: sweeping orphan [" << filePath << ']');
                    FileUtil::removeFile(filePath, /*recursive=*/true);
                }
                continue;
            }

            version.size = fileStat.size();
            entry.versions.push_back(std::move(version));
        }

        if (entry.versions.empty())
        {
            // meta.json alone is just a cache.
            FileUtil::removeFile(dirPath, /*recursive=*/true);
            continue;
        }

        sortVersions(entry);
        TotalSizeBytes += entry.size();
        Index.emplace(dirName, std::move(entry));
    }
}

bool ensureDirectory(const std::string& path)
{
    try
    {
        FileUtil::createDirectories(path);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        LOG_ERR("VersionStore: cannot create directory [" << path << "]: " << e.what());
        return false;
    }
#ifndef _WIN32
    ::chmod(path.c_str(), S_IRWXU);
#endif
    return true;
}

/// Copy @docPath to @destPath and verify the source did not change
/// underneath the copy: the engine's fallback save path rewrites files
/// in place (see engine sfx2 SfxMedium::Transfer_Impl), and
/// FileUtil::copy warn-and-succeeds when the size changes mid-copy.
bool copyVerified(const std::string& docPath, const std::string& destPath)
{
    for (int attempt = 0; attempt < 2; ++attempt)
    {
        const FileUtil::Stat before(docPath);
        if (!before.good())
            return false;

        if (!FileUtil::copyAtomic(docPath, destPath, /*preserveTimestamps=*/true))
            return false;

        const FileUtil::Stat after(docPath);
        if (after.good() && after.size() == before.size() &&
            after.modifiedTime().tv_sec == before.modifiedTime().tv_sec &&
            after.modifiedTime().tv_nsec == before.modifiedTime().tv_nsec)
            return true;

        LOG_WRN("VersionStore: [" << docPath << "] changed while copying; "
                                  << (attempt == 0 ? "retrying" : "giving up"));
        FileUtil::removeFile(destPath);
    }
    return false;
}

} // anonymous namespace

std::string formatTimestamp(std::chrono::system_clock::time_point tp)
{
    const std::time_t time = std::chrono::system_clock::to_time_t(tp);
    std::tm tm;
    Util::time_t_to_gmtime(time, tm);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d-%H%M%S", &tm);
    return buffer;
}

bool parseVersionFilename(const std::string& fileName,
                          std::chrono::system_clock::time_point& timestamp)
{
    unsigned year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
    int consumed = 0;
    if (std::sscanf(fileName.c_str(), "%4u%2u%2u-%2u%2u%2u%n", &year, &month, &day, &hour,
                    &minute, &second, &consumed) != 6 ||
        consumed != 15)
        return false;

    if (month < 1 || month > 12 || day < 1 || day > 31 || hour > 23 || minute > 59 ||
        second > 60)
        return false;

    // The remainder must be "[-N].<ext>" with a short alphanumeric
    // extension: a copyAtomic temporary is the final name plus 12 random
    // characters, which overflows the extension and correctly fails to
    // parse, so the startup sweep catches it.
    std::size_t pos = consumed;
    if (pos < fileName.size() && fileName[pos] == '-')
    {
        ++pos;
        const std::size_t digitsBegin = pos;
        while (pos < fileName.size() && std::isdigit(static_cast<unsigned char>(fileName[pos])))
            ++pos;
        if (pos == digitsBegin)
            return false;
    }
    if (pos >= fileName.size() || fileName[pos] != '.')
        return false;
    const std::size_t extension = fileName.size() - pos - 1;
    if (extension < 1 || extension > 10)
        return false;
    for (++pos; pos < fileName.size(); ++pos)
        if (!std::isalnum(static_cast<unsigned char>(fileName[pos])))
            return false;

    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    const time_t time = timegmPortable(tm);
    if (time == static_cast<time_t>(-1))
        return false;

    timestamp = std::chrono::system_clock::from_time_t(time);
    return true;
}

void initialize(const std::string& rootPath, const Limits& limits,
                std::vector<std::string> excludedPathPrefixes,
                const std::string& exportDirectory)
{
    std::scoped_lock lock(StoreMutex);

    StoreLimits = limits;
    RootPath = rootPath;
    ExportDir = exportDirectory;
    ExcludedPrefixes = std::move(excludedPathPrefixes);
    InFlight.clear();
    Pending.clear();
    TooLargeWarned.clear();
    Initialized = !RootPath.empty();
    if (!Initialized)
        return;

    if (StoreLimits.enabled && !ensureDirectory(RootPath))
    {
        Initialized = false;
        return;
    }

    const auto now = std::chrono::system_clock::now();
    scanStore(now);
    pruneLocked(now);

    LOG_INF("VersionStore: initialized at [" << RootPath << "] with " << Index.size()
                                             << " documents, "
                                             << Util::getHumanizedBytes(TotalSizeBytes));
}

bool isEnabled()
{
    std::scoped_lock lock(StoreMutex);
    return Initialized && StoreLimits.enabled;
}

SnapshotResult snapshotSync(const std::string& docPath, bool bypassMergeWindow)
{
    std::unique_lock lock(StoreMutex);

    if (!Initialized || !StoreLimits.enabled)
        return SnapshotResult::Disabled;

    const std::string docKey = FileUtil::hostPath(docPath);
    if (isExcluded(docPath, docKey))
        return SnapshotResult::Excluded;

    const FileUtil::Stat source(docPath);
    if (!source.isFile())
        return SnapshotResult::Failed;

    // A document over the store cap would evict every other document and
    // still not fit.
    const bool overFileLimit = source.size() > StoreLimits.maxFileSizeBytes;
    if (overFileLimit || source.size() > StoreLimits.maxStoreSizeBytes)
    {
        if (TooLargeWarned.insert(docKey).second)
            LOG_INF("VersionStore: not versioning ["
                    << docKey << "]: " << Util::getHumanizedBytes(source.size()) << " exceeds the "
                    << (overFileLimit ? "file size limit of " : "store size limit of ")
                    << Util::getHumanizedBytes(overFileLimit ? StoreLimits.maxFileSizeBytes
                                                             : StoreLimits.maxStoreSizeBytes));
        return SnapshotResult::TooLarge;
    }

    const std::string dirName = hashDirName(docKey);
    const std::string displayName = fileNameOf(docKey);
    const auto now = std::chrono::system_clock::now();

    const auto newestName = [&dirName]()
    {
        const auto it = Index.find(dirName);
        return it == Index.end() || it->second.versions.empty()
                   ? std::string()
                   : it->second.versions.back().fileName;
    };
    // The result when the newest version makes a version of @path needless.
    const auto needless = [&](const FileUtil::Stat& stat,
                              const std::string& path) -> std::optional<SnapshotResult>
    {
        const auto it = Index.find(dirName);
        if (it == Index.end() || it->second.versions.empty())
            return std::nullopt;

        const VersionInfo& newest = it->second.versions.back();
        if (!bypassMergeWindow && now - newest.timestamp < StoreLimits.mergeWindow)
            return SnapshotResult::Coalesced;

        const std::string newestPath = versionPath(dirName, newest.fileName);
        if (FileUtil::Stat::isIdenticalTo(FileUtil::Stat(newestPath), newestPath, stat, path))
            return SnapshotResult::Deduplicated;
        return std::nullopt;
    };

    if (const std::optional<SnapshotResult> result = needless(source, docPath))
        return *result;

    // Off the apps the check asks the server unit-test hook, which only a server has.
    if (Util::isMobileApp() && !FileUtil::checkDiskSpace(RootPath))
    {
        LOG_WRN("VersionStore: not versioning [" << docKey << "]: low disk space");
        return SnapshotResult::Failed;
    }

    // The copy can take long, and the lock would hold up every other user
    // of the store, the UI thread among them. The copy goes to a file in
    // the root, which is no version of any document, and the store takes
    // it under the lock afterwards. initialize() sweeps a file left there.
    const std::string rootPath = RootPath;
    const std::string newestBefore = newestName();
    const std::string tempPath = rootPath + '/' + dirName + Util::rng::getFilename(12);
    const std::function<void()> hook = CopyHookForTest;

    lock.unlock();
    bool copied = false;
    try
    {
        copied = copyVerified(docPath, tempPath);
    }
    catch (const std::exception& e)
    {
        LOG_ERR("VersionStore: copy of [" << docKey << "] failed: " << e.what());
    }
    if (hook)
        hook();
    lock.lock();

    const FileUtil::Stat temp(tempPath);
    if (!copied || !temp.isFile())
    {
        LOG_ERR("VersionStore: failed to snapshot [" << docKey << ']');
        return SnapshotResult::Failed;
    }

    // The store can have changed during the copy.
    std::optional<SnapshotResult> dropped;
    if (!Initialized || !StoreLimits.enabled || RootPath != rootPath)
        dropped = SnapshotResult::Disabled;
    else if (newestName() != newestBefore)
        dropped = needless(temp, tempPath);
    if (dropped)
    {
        FileUtil::removeFile(tempPath);
        return *dropped;
    }

    // May evict from any document, including this one; take the entry
    // reference only afterwards.
    makeSpace(temp.size());
    DocEntry& entry = Index[dirName];

    // The extension must fit the version-filename grammar (see
    // parseVersionFilename).
    std::string extension = FileUtil::extractFileExtension(docPath);
    const auto isAlnum = [](unsigned char c) { return std::isalnum(c) != 0; };
    if (extension.empty() || extension.size() > 10 ||
        !std::all_of(extension.begin(), extension.end(), isAlnum))
        extension = "dat";

    const std::string stamp = formatTimestamp(now);
    std::string fileName = stamp + '.' + extension;
    const auto exists = [&entry](const std::string& name)
    {
        return std::any_of(entry.versions.begin(), entry.versions.end(),
                           [&name](const VersionInfo& version)
                           { return version.fileName == name; });
    };
    for (int counter = 1; exists(fileName) ||
                          FileUtil::Stat(versionPath(dirName, fileName)).exists();
         ++counter)
        fileName = stamp + '-' + std::to_string(counter) + '.' + extension;

    const std::string destPath = versionPath(dirName, fileName);
    if (!ensureDirectory(docDirPath(dirName)) ||
        std::rename(tempPath.c_str(), destPath.c_str()) != 0)
    {
        LOG_ERR("VersionStore: failed to snapshot [" << docKey << ']');
        FileUtil::removeFile(tempPath);
        if (entry.versions.empty())
            removeDocDir(dirName);
        return SnapshotResult::Failed;
    }
    if (entry.docKey.empty() || entry.displayName != displayName)
    {
        entry.docKey = docKey;
        entry.displayName = displayName;
        writeMeta(dirName, entry);
    }

    VersionInfo version;
    version.fileName = fileName;
    version.timestamp = now;
    version.size = temp.size();
    TotalSizeBytes += version.size;
    entry.versions.push_back(std::move(version));
    // A copy that started later can end first.
    sortVersions(entry);

    LOG_DBG("VersionStore: snapshotted [" << docKey << "] as [" << fileName << ']');

    thinDoc(dirName, entry, now);
    return SnapshotResult::Created;
}

void setCopyHookForTest(std::function<void()> hook)
{
    std::scoped_lock lock(StoreMutex);
    CopyHookForTest = std::move(hook);
}

void snapshot(std::string docPath, bool bypassMergeWindow)
{
    {
        std::scoped_lock lock(StoreMutex);
        if (!Initialized || !StoreLimits.enabled)
            return;
        if (!InFlight.insert(docPath).second)
        {
            Pending[docPath] |= bypassMergeWindow;
            return;
        }
    }

    std::thread(
        [docPath = std::move(docPath), bypassMergeWindow]() mutable
        {
            for (;;)
            {
                // A detached thread has nothing above it to catch; a lost
                // snapshot beats a terminated app.
                try
                {
                    snapshotSync(docPath, bypassMergeWindow);
                }
                catch (const std::exception& e)
                {
                    LOG_ERR("VersionStore: snapshot of [" << docPath << "] failed: " << e.what());
                }
                std::scoped_lock lock(StoreMutex);
                const auto it = Pending.find(docPath);
                if (it == Pending.end())
                {
                    InFlight.erase(docPath);
                    return;
                }
                bypassMergeWindow = it->second;
                Pending.erase(it);
            }
        })
        .detach();
}

std::vector<VersionInfo> listVersions(const std::string& docPath)
{
    std::scoped_lock lock(StoreMutex);
    if (!Initialized)
        return {};

    const auto it = Index.find(hashDirName(FileUtil::hostPath(docPath)));
    if (it == Index.end())
        return {};

    return std::vector<VersionInfo>(it->second.versions.rbegin(), it->second.versions.rend());
}

std::string exportVersion(const std::string& docPath, const std::string& versionFileName)
{
    std::scoped_lock lock(StoreMutex);
    if (!Initialized || ExportDir.empty() ||
        versionFileName.find('/') != std::string::npos)
        return {};

    const std::string dirName = hashDirName(FileUtil::hostPath(docPath));
    const auto it = Index.find(dirName);
    if (it == Index.end())
        return {};

    const auto versionIt =
        std::find_if(it->second.versions.begin(), it->second.versions.end(),
                     [&versionFileName](const VersionInfo& version)
                     { return version.fileName == versionFileName; });
    if (versionIt == it->second.versions.end())
        return {};

    // "report.odt" and 2026-08-06T15:30Z become "report (2026-08-06 15.30).odt".
    std::string base = it->second.displayName.empty() ? fileNameOf(docPath)
                                                      : it->second.displayName;
    std::string extension;
    const std::size_t dot = base.find_last_of('.');
    if (dot != std::string::npos && dot > 0)
    {
        extension = base.substr(dot);
        base = base.substr(0, dot);
    }

    const std::time_t time = std::chrono::system_clock::to_time_t(versionIt->timestamp);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    char stamp[32];
    std::strftime(stamp, sizeof(stamp), "%Y-%m-%d %H.%M", &tm);

    if (!ensureDirectory(ExportDir))
        return {};
    std::string destPath = ExportDir + '/' + base + " (" + stamp + ')' + extension;
    for (int counter = 2; FileUtil::Stat(destPath).exists(); ++counter)
        destPath = ExportDir + '/' + base + " (" + stamp + ") (" + std::to_string(counter) +
                   ')' + extension;

    if (!FileUtil::copyAtomic(versionPath(dirName, versionFileName), destPath,
                              /*preserveTimestamps=*/false))
    {
        LOG_ERR("VersionStore: failed to export [" << versionFileName << ']');
        return {};
    }

    return destPath;
}

void clearHistory(const std::string& docPath)
{
    std::scoped_lock lock(StoreMutex);
    if (!Initialized)
        return;

    removeDocDir(hashDirName(FileUtil::hostPath(docPath)));
}

std::uint64_t storeSizeBytes()
{
    std::scoped_lock lock(StoreMutex);
    return TotalSizeBytes;
}

void pruneNow(std::chrono::system_clock::time_point now)
{
    std::scoped_lock lock(StoreMutex);
    if (Initialized)
        pruneLocked(now);
}

std::string docDirectory(const std::string& docPath)
{
    std::scoped_lock lock(StoreMutex);
    if (!Initialized)
        return {};
    return docDirPath(hashDirName(FileUtil::hostPath(docPath)));
}

namespace policy
{

std::vector<std::size_t> thin(const std::vector<VersionInfo>& newestFirst,
                              std::chrono::system_clock::time_point now, std::size_t maxCount)
{
    using std::chrono::hours;
    using std::chrono::seconds;

    const std::size_t count = newestFirst.size();
    // The newest few survive tiering and the count cap no matter what
    // their timestamps claim: a clock jump must not delete the most
    // recent versions.
    constexpr std::size_t ProtectedNewest = 3;

    std::vector<bool> keep(count, true);
    std::set<std::int64_t> hourly, daily, weekly, monthly;
    for (std::size_t i = 0; i < count; ++i)
    {
        const seconds age =
            std::max(seconds::zero(), std::chrono::duration_cast<seconds>(
                                          now - newestFirst[i].timestamp));
        const std::int64_t epoch =
            std::chrono::duration_cast<seconds>(newestFirst[i].timestamp.time_since_epoch())
                .count();

        // Newest-first, the first version seen in a bucket survives.
        // Protected versions still register their buckets so an older
        // sibling of the same bucket does not also survive.
        bool tierKeep = true;
        if (age < hours(1))
            tierKeep = true;
        else if (age < hours(24))
            tierKeep = hourly.insert(epoch / 3600).second;
        else if (age < hours(24 * 30))
            tierKeep = daily.insert(epoch / (24 * 3600)).second;
        else if (age < hours(24 * 91))
            tierKeep = weekly.insert(epoch / (7 * 24 * 3600)).second;
        else if (age < hours(24 * 365))
            tierKeep = monthly.insert(epoch / (30LL * 24 * 3600)).second;
        else
            tierKeep = false;

        if (i >= ProtectedNewest)
            keep[i] = tierKeep;
    }

    std::vector<std::size_t> survivors;
    for (std::size_t i = 0; i < count; ++i)
        if (keep[i])
            survivors.push_back(i);

    while (survivors.size() > maxCount)
    {
        // Drop the survivor closest to its next-newer neighbour: dense
        // recent bursts collapse first, the sparse old tail survives.
        std::size_t victim = survivors.size();
        seconds smallestGap = seconds::max();
        for (std::size_t s = 1; s < survivors.size(); ++s)
        {
            if (survivors[s] < ProtectedNewest)
                continue;
            const seconds gap = std::chrono::duration_cast<seconds>(
                newestFirst[survivors[s - 1]].timestamp - newestFirst[survivors[s]].timestamp);
            if (gap < smallestGap)
            {
                smallestGap = gap;
                victim = s;
            }
        }

        if (victim == survivors.size())
            break; // Nothing eligible.

        keep[survivors[victim]] = false;
        survivors.erase(survivors.begin() + victim);
    }

    std::vector<std::size_t> deletions;
    for (std::size_t i = 0; i < count; ++i)
        if (!keep[i])
            deletions.push_back(i);
    return deletions;
}

} // namespace policy

} // namespace versioning

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
