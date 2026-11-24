/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Implementation of file quarantine mechanism.
 * Functions: quarantineFile()
 */

#include <config.h>

#include "QuarantineUtil.hpp"

#include <common/Common.hpp>
#include <common/FileUtil.hpp>
#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/StringVector.hpp>
#include <common/Util.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/DocumentBroker.hpp>

#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/URI.h>

#include <algorithm>
#include <chrono>
#include <exception>
#include <fstream>
#include <mutex>
#include <stdexcept>

namespace
{
std::size_t getSecondsSinceEpoch()
{
    const auto timeNow = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(timeNow.time_since_epoch()).count();
}

/// Whether quarantine_files is enabled in the active config. A value that can't be read is false.
bool isEnabledInConfig()
{
    try
    {
        return ConfigUtil::getBool("quarantine_files[@enable]", false);
    }
    catch (const std::exception& exc)
    {
        LOG_WRN("Invalid quarantine_files[@enable], the quarantine is off: " << exc.what());
    }

    return false;
}

/// A setting of the quarantine from the active config, a number that is not negative. Where
/// that can't be read, the default.
std::size_t getSetting(const std::string& key, int def)
{
    try
    {
        const int value = ConfigUtil::getInt(key, def);
        if (value >= 0)
            return static_cast<std::size_t>(value);

        LOG_WRN("Invalid " << key << " of " << value << ", taking " << def);
    }
    catch (const std::exception& exc)
    {
        LOG_WRN("Invalid " << key << ", taking " << def << ": " << exc.what());
    }

    return static_cast<std::size_t>(def);
}

} // namespace

std::string Quarantine::QuarantinePath;
Util::UnorderedStringMap<std::vector<Quarantine::Entry>> Quarantine::QuarantineMap;
std::unordered_set<std::string> Quarantine::OpenDocKeys;
std::mutex Quarantine::Mutex;
std::size_t Quarantine::MaxSizeBytes;
std::size_t Quarantine::MaxAgeSecs;
std::size_t Quarantine::MaxVersions;

Quarantine::Quarantine(DocumentBroker& docBroker, const std::string& docName)
    : _docKey(docBroker.getDocKey())
    , _docName(Uri::encode(docName, std::string(",/?:@&=+$#") + Delimiter))
    , _quarantinedFilename(Delimiter + std::to_string(docBroker.getPid()) + Delimiter + _docName)
{
    std::string anonymizedFilename = _quarantinedFilename;
    Util::replaceAllSubStr(anonymizedFilename, _docName, Anonymizer::anonymize(_docName));
    LOG_DBG("Quarantine ctor for [" << _docKey << "], filename: [" << anonymizedFilename << ']');
}

void Quarantine::initialize(const std::string& path)
{
    // The active config, which a test can set without an application.
    if (!QuarantinePath.empty() || !isEnabledInConfig())
    {
        return;
    }

    MaxSizeBytes = getSetting("quarantine_files.limit_dir_size_mb", 250) * 1024 * 1024;
    MaxAgeSecs = getSetting("quarantine_files.expiry_min", 3000) * 60;
    MaxVersions = std::max<std::size_t>(getSetting("quarantine_files.max_versions_to_maintain", 5),
                                        1);
    LOG_INF("Initializing Quarantine at [" << path << "] with Max Size: " << MaxSizeBytes
                                           << " bytes, Max Age: " << MaxAgeSecs
                                           << " seconds, Max Versions: " << MaxVersions);

    // Make sure the quarantine directories exists, or we throw if we can't create it.
    try
    {
        Poco::File(path).createDirectories();
    }
    catch (const std::exception&)
    {
        LOG_FTL("Quarantine directory [" << path
                                         << "] is invalid or we have no permission to create it");
        throw;
    }

    // Make sure we can write into the quarantine directory.
    {
        const std::string testFile = "quarantine.test";
        const Poco::Path target(path, testFile);
        const std::string testPath = target.toString();

        FileUtil::removeFile(testPath); // Make sure there are no left-overs.

        try
        {
            Poco::File file(target);
            file.createFile();
            if (!FileUtil::Stat(testPath).exists())
            {
                throw std::runtime_error("Cannot write to quarantine directory [" + path +
                                         "] as it is read-only");
            }
        }
        catch (const std::exception&)
        {
            LOG_FTL("Quarantine directory [" << path
                                             << "] is read-only. Please ensure that the coolwsd "
                                                "process account has write permissions to it");
            throw;
        }

        FileUtil::removeFile(testPath); // Make sure there are no left-overs.
    }

    // This function should ever be called once, but for consistency, take the lock.
    std::lock_guard<std::mutex> lock(Mutex);

    QuarantineMap.clear();
    OpenDocKeys.clear();

    std::vector<Poco::File> legacyFiles;
    std::vector<Poco::File> files;
    Poco::File(path).list(files);
    for (const Poco::File& file : files)
    {
        if (file.isFile())
        {
            legacyFiles.emplace_back(file);
        }
        else if (file.isDirectory())
        {
            // Directories are always DocKeys.
            Poco::Path filePath = file.path();
            const std::string& docKey = filePath.directory(filePath.depth());
            const std::string fullPath = file.path();

            std::vector<Poco::File> newFiles;
            Poco::File(fullPath).list(newFiles);
            if (newFiles.empty())
            {
                // Remove empty directories.
                LOG_TRC("Removing empty quarantine directory [" << fullPath << ']');
                FileUtil::removeFile(fullPath, /*recursive=*/true);
                continue;
            }

            std::vector<Entry> entries;
            entries.reserve(newFiles.size());
            for (const Poco::File& newFile : newFiles)
            {
                if (newFile.isFile())
                {
                    const std::string fileName = Poco::Path(newFile.path()).getFileName();
                    if (fileName.starts_with(CopyPrefix))
                    {
                        // A copy that was not finished when wsd went away.
                        LOG_DBG("Removing unfinished quarantine copy [" << newFile.path() << ']');
                        FileUtil::removeFile(newFile.path());
                    }
                    else if (fileName != StateFileName)
                        entries.emplace_back(path, docKey, fileName);
                }
            }

            LOG_TRC("Found " << entries.size() << " quarantine file for DocKey [" << docKey << ']');
            if (!entries.empty())
            {
                readState(path, docKey, entries);
                QuarantineMap[docKey] = std::move(entries);
            }
            else
            {
                LOG_TRC("Removing quarantine directory without versions [" << fullPath << ']');
                FileUtil::removeFile(fullPath, /*recursive=*/true);
            }
        }
    }

    LOG_TRC("Found " << legacyFiles.size() << " quarantine legacy files");
    for (const Poco::File& legacyFile : legacyFiles)
    {
        Entry entry(path, Poco::Path(legacyFile.path()).getFileName());

        QuarantineMap[entry.docKey()].emplace_back(entry);
    }

    // Now we need to sort the files for each DocKey from oldest to newest.
    for (auto& pair : QuarantineMap)
    {
        std::sort(pair.second.begin(), pair.second.end(), [](const auto& lhs, const auto& rhs)
                  { return lhs.secondsSinceEpoch() < rhs.secondsSinceEpoch(); });
    }

    // We are initialized at this point.
    QuarantinePath = path;

    for (const auto& pair : QuarantineMap)
    {
        LOG_TRC("BC Found " << pair.second.size() << " quarantine file(s) for DocKey ["
                            << pair.first << ']');
    }

    // Clean up, and write the state of each document again, with the protection it has now.
    makeQuarantineSpace(/*headroomBytes=*/0, /*mayRemoveProtected=*/true);
    for (const auto& pair : QuarantineMap)
    {
        try
        {
            writeState(pair.first);
        }
        catch (const std::exception& exc)
        {
            LOG_WRN("Failed to write the quarantine state of [" << pair.first
                                                                << "]: " << exc.what());
        }
    }
    removeEmptyDocKeys();

    for (const auto& pair : QuarantineMap)
    {
        LOG_TRC("AC Found " << pair.second.size() << " quarantine file(s) for DocKey ["
                            << pair.first << ']');
    }

    LOG_DBG("Found " << QuarantineMap.size() << " DocKey quarantines with total "
                     << quarantineSize() << " bytes");
}

std::size_t Quarantine::quarantineSize()
{
    LOG_ASSERT_MSG(!Mutex.try_lock(), "Quarantine Mutex must be taken");

    std::size_t size = 0;
    for (const auto& pair : QuarantineMap)
    {
        for (const Entry& entry : pair.second)
        {
            size += entry.size();
        }
    }

    return size;
}

bool Quarantine::makeQuarantineSpace(std::size_t headroomBytes, bool mayRemoveProtected)
{
    LOG_ASSERT_MSG(!Mutex.try_lock(), "Quarantine Mutex must be taken");

    if (!isEnabled())
        return false;

    // Start with culling the expired and excess versions.
    const std::size_t now = getSecondsSinceEpoch();
    for (const auto& pair : QuarantineMap)
    {
        deleteOldQuarantineVersions(pair.first, now > MaxAgeSecs ? now - MaxAgeSecs : 0);
    }

    std::size_t currentSize = quarantineSize() + headroomBytes;
    if (currentSize <= MaxSizeBytes)
        return true;

    // When the protected versions are to stay, check first that there can be room at all, so
    // the other versions are not removed for an incoming file that does not fit anyway.
    std::size_t keptBytes = headroomBytes;
    if (!mayRemoveProtected)
    {
        for (const auto& pair : QuarantineMap)
        {
            for (const Entry& entry : pair.second)
            {
                if (entry.isProtected())
                    keptBytes += entry.size();
            }
        }
    }

    if (keptBytes > MaxSizeBytes)
    {
        LOG_WRN("Quarantine directory has no room for an incoming file of "
                << headroomBytes << " bytes"
                << (mayRemoveProtected ? "" : " without removing protected versions"));
        return false;
    }

    LOG_INF("Quarantine directory is too large at "
            << currentSize - headroomBytes << " bytes for incoming file of " << headroomBytes
            << " bytes (Max Size: " << MaxSizeBytes << " bytes). Shrinking.");

    while (currentSize > MaxSizeBytes)
    {
        // The version to remove: of the document with the most versions that are not protected,
        // the oldest of those; when no such version is left, the oldest protected version.
        std::vector<Entry>* victimList = nullptr;
        std::size_t victimIndex = 0;
        std::size_t victimCount = 0;
        for (auto& pair : QuarantineMap)
        {
            std::vector<Entry>& entries = pair.second;
            std::size_t unprotected = 0;
            std::size_t oldest = entries.size();
            for (std::size_t i = 0; i < entries.size(); ++i)
            {
                if (entries[i].isProtected())
                    continue;

                if (oldest == entries.size())
                    oldest = i;
                ++unprotected;
            }

            if (unprotected > 0 &&
                (unprotected > victimCount ||
                 (unprotected == victimCount && entries[oldest].secondsSinceEpoch() <
                                                    (*victimList)[victimIndex].secondsSinceEpoch())))
            {
                victimList = &entries;
                victimIndex = oldest;
                victimCount = unprotected;
            }
        }

        if (!victimList && mayRemoveProtected)
        {
            for (auto& pair : QuarantineMap)
            {
                std::vector<Entry>& entries = pair.second;
                for (std::size_t i = 0; i < entries.size(); ++i)
                {
                    if (!entries[i].isProtected())
                        continue;

                    if (!victimList || entries[i].secondsSinceEpoch() <
                                           (*victimList)[victimIndex].secondsSinceEpoch())
                    {
                        victimList = &entries;
                        victimIndex = i;
                    }

                    break; // The oldest protected version of this document.
                }
            }
        }

        if (!victimList)
        {
            LOG_WRN("Quarantine directory has no room for an incoming file of " << headroomBytes
                                                                                << " bytes");
            return false;
        }

        const Entry& entry = (*victimList)[victimIndex];
        const std::size_t size = entry.size();
        const bool wasProtected = entry.isProtected();
        const std::string docKey = entry.docKey();
        LOG_DBG("Removing quarantined file [" << entry.filename() << "] for [" << docKey
                                              << "] with " << size << " bytes"
                                              << (wasProtected ? ", protected" : ""));
        FileUtil::removeFile(entry.fullPath());
        victimList->erase(victimList->begin() + victimIndex); // Invalidates entry.
        currentSize -= std::min(size, currentSize);

        if (wasProtected)
            writeState(docKey);
    }

    return true;
}

void Quarantine::removeEmptyDocKeys()
{
    LOG_ASSERT_MSG(!Mutex.try_lock(), "Quarantine Mutex must be taken");

    for (auto it = QuarantineMap.begin(); it != QuarantineMap.end();)
    {
        // A legacy version whose name could not be read has no docKey; it is in no directory.
        if (!it->second.empty() || OpenDocKeys.contains(it->first) || it->first.empty())
        {
            ++it;
            continue;
        }

        const std::string docKeyPath = Poco::Path(QuarantinePath, it->first).toString();
        FileUtil::removeFile(Poco::Path(docKeyPath, StateFileName).toString());
        if (FileUtil::isEmptyDirectory(docKeyPath))
            FileUtil::removeFile(docKeyPath, /*recursive=*/true);

        it = QuarantineMap.erase(it);
    }
}

void Quarantine::writeState(const std::string& docKey)
{
    LOG_ASSERT_MSG(!Mutex.try_lock(), "Quarantine Mutex must be taken");

    const bool isOpen = OpenDocKeys.contains(docKey);
    std::vector<std::string> protectedNames;
    for (const Entry& entry : QuarantineMap[docKey])
    {
        if (entry.isProtected())
            protectedNames.emplace_back(Poco::Path(entry.fullPath()).getFileName());
    }

    const std::string docKeyPath = Poco::Path(QuarantinePath, docKey).toString();
    const std::string statePath = Poco::Path(docKeyPath, StateFileName).toString();
    if (!isOpen && protectedNames.empty())
    {
        FileUtil::removeFile(statePath);
        return;
    }

    // The state is written in full under another name first, so it is never found half written.
    // That name is one of an unfinished copy, which goes at the next start.
    Poco::File(docKeyPath).createDirectories();
    const std::string newStatePath =
        Poco::Path(docKeyPath, std::string(CopyPrefix) + StateFileName).toString();
    {
        std::ofstream state(newStatePath, std::ios::trunc);
        state << (isOpen ? "open" : "closed") << '\n';
        for (const std::string& name : protectedNames)
            state << name << '\n';

        state.flush();
        if (!state)
        {
            LOG_WRN("Failed to write the quarantine state of [" << docKey << ']');
            FileUtil::removeFile(newStatePath);
            return;
        }
    }

    if (::rename(newStatePath.c_str(), statePath.c_str()) != 0)
    {
        LOG_SYS("Failed to write the quarantine state of [" << docKey << ']');
        FileUtil::removeFile(newStatePath);
    }
}

void Quarantine::readState(const std::string& root, const std::string& docKey,
                           std::vector<Entry>& entries)
{
    const std::string statePath =
        Poco::Path(Poco::Path(root, docKey).toString(), StateFileName).toString();
    std::ifstream state(statePath);
    if (!state)
        return;

    std::string line;
    std::getline(state, line);

    // A document that was still open when wsd went away ended badly.
    const bool wasOpen = line == "open";
    std::unordered_set<std::string> protectedNames;
    while (std::getline(state, line))
        protectedNames.insert(line);

    for (Entry& entry : entries)
    {
        if (wasOpen || protectedNames.contains(Poco::Path(entry.fullPath()).getFileName()))
            entry.setProtected(true);
    }

    LOG_DBG("Quarantine state of [" << docKey << "]: " << (wasOpen ? "was open" : "closed")
                                    << ", " << protectedNames.size() << " protected versions");
}

void Quarantine::deleteOldQuarantineVersions(const std::string& docKey, std::size_t oldestTimestamp)
{
    LOG_ASSERT_MSG(!Mutex.try_lock(), "Quarantine Mutex must be taken");

    if (!isEnabled())
        return;

    std::vector<Entry>& container = QuarantineMap[docKey];

    // The versions are sorted oldest first. Of those that are not protected, the oldest go when
    // there are more than MaxVersions of them.
    std::size_t unprotected = 0;
    for (const Entry& entry : container)
    {
        if (!entry.isProtected())
            ++unprotected;
    }

    std::size_t excessVersions = unprotected > MaxVersions ? unprotected - MaxVersions : 0;
    bool protectedRemoved = false;
    std::size_t removed = 0;
    for (std::size_t i = 0; i < container.size();)
    {
        const Entry& entry = container[i];
        const bool expired = entry.secondsSinceEpoch() < oldestTimestamp;
        const bool excess = !entry.isProtected() && excessVersions > 0;
        if (!expired && !excess)
        {
            ++i;
            continue;
        }

        LOG_TRC("Removing " << (expired ? "expired" : "excess") << " quarantined-file version ["
                            << entry.fullPath() << "] for [" << docKey << ']');
        FileUtil::removeFile(entry.fullPath());
        if (!entry.isProtected())
            excessVersions -= std::min<std::size_t>(excessVersions, 1);
        else
            protectedRemoved = true;

        container.erase(container.begin() + i);
        ++removed;
    }

    if (removed > 0)
        LOG_DBG("Removed " << removed << " expired or excess quarantined-file versions for ["
                           << docKey << "], " << container.size() << " left");

    if (protectedRemoved)
        writeState(docKey);
}

bool Quarantine::quarantineFile(const std::string& docPath, bool copy)
{
    try
    {
        return quarantineFile(_docKey, docPath, _quarantinedFilename, copy, _endedBadly);
    }
    catch (const std::exception& exc)
    {
        LOG_WRN("Failed to quarantine [" << docPath << "] for docKey [" << _docKey
                                         << "]: " << exc.what());
    }

    return false;
}

bool Quarantine::quarantineFile(const std::string& docKey, const std::string& docPath,
                                const std::string& quarantinedFilename, bool copy,
                                bool isProtected)
{
    if (!isEnabled())
        return false;

    FileUtil::Stat sourceStat(docPath);
    if (!sourceStat.exists())
    {
        LOG_WRN("Quarantining of file [" << docPath << "] failed because it does not exist");
        return false;
    }

    // A version larger than the whole quarantine would push out every other version, and then go
    // over the limit anyway.
    if (sourceStat.size() > MaxSizeBytes)
    {
        LOG_WRN("Quarantining of file [" << docPath << "] is skipped because its "
                                         << sourceStat.size()
                                         << " bytes are more than the quarantine can hold");
        return false;
    }

    const std::string docKeyPath = Poco::Path(QuarantinePath, docKey).toString();

    // The versions of one document are quarantined by its own broker, one at a time. The lock is
    // shared by all documents, so reading and copying the file, which takes as long as the file is
    // big, happen without it.
    std::string lastFile;
    {
        std::lock_guard<std::mutex> lock(Mutex);
        const std::vector<Entry>& fileList = QuarantineMap[docKey];
        if (!fileList.empty())
            lastFile = fileList[fileList.size() - 1].fullPath();
    }

    // Check if we have a duplicate or a new version. The document is in use again, so a duplicate
    // ages from now, like a new version would. When the duplicate went meanwhile, to make space,
    // the file is quarantined after all.
    if (!lastFile.empty() &&
        FileUtil::Stat::isIdenticalTo(FileUtil::Stat(lastFile), lastFile, sourceStat, docPath) &&
        renewVersion(docKey, lastFile))
    {
        LOG_INF("Quarantining of file ["
                << docPath << "] is skipped because this file version is already quarantined as ["
                << lastFile << ']');
        return false;
    }

    // The new version is made under another name first, and that is removed unless it becomes
    // the new version, also when anything throws.
    struct UnfinishedCopy
    {
        std::string path;
        ~UnfinishedCopy()
        {
            if (!path.empty())
                FileUtil::removeFile(path);
        }
    } unfinishedCopy;

    // The directory is made under the lock, as removeEmptyDocKeys() removes directories under it
    // too.
    {
        std::lock_guard<std::mutex> lock(Mutex);
        Poco::File(docKeyPath).createDirectories();
    }

    // Room for the version is made only once it is there, so a version that can't be made, for
    // example because its file went meanwhile, costs no other version.
    const std::string copyPath =
        Poco::Path(docKeyPath, CopyPrefix + Util::rng::getFilename(12)).toString();
    unfinishedCopy.path = copyPath;
    const bool made = copy ? FileUtil::copy(docPath, copyPath, /*log=*/false,
                                            /*throw_on_error=*/false)
                           : FileUtil::linkOrCopyFile(docPath, copyPath);
    if (!made)
    {
        LOG_ERR("Quarantining of file [" << docPath << "] failed to " << (copy ? "copy" : "link")
                                         << " it to [" << copyPath << ']');
        return false;
    }

    std::lock_guard<std::mutex> lock(Mutex);

    // The time is part of the file name, so a second version within the same second would replace
    // the first. Name it a second later instead, which keeps the versions in order.
    std::vector<Entry>& fileList = QuarantineMap[docKey];
    uint64_t secondsSinceEpoch = getSecondsSinceEpoch();
    if (!fileList.empty())
        secondsSinceEpoch =
            std::max(secondsSinceEpoch, fileList[fileList.size() - 1].secondsSinceEpoch() + 1);

    Entry entry(QuarantinePath, docKey, secondsSinceEpoch, quarantinedFilename,
                sourceStat.size());
    entry.setProtected(isProtected);
    const std::string linkedFilePath = entry.fullPath();
    LOG_TRC("Quarantining [" << docPath << "] to [" << linkedFilePath << ']');

    // Clean-up the quarantine directory and make room for the incoming file.
    bool quarantined = makeQuarantineSpace(entry.size(), isProtected);
    if (quarantined)
    {
        Poco::File(docKeyPath).createDirectories();
        quarantined = ::rename(copyPath.c_str(), linkedFilePath.c_str()) == 0;
    }

    if (quarantined)
    {
        unfinishedCopy.path.clear();
        fileList.emplace_back(entry);
        LOG_INF("Quarantined [" << docPath << "] to [" << linkedFilePath << ']'
                                << (isProtected ? ", protected" : ""));

        // The space above was made before this version was added, so it may be one too many.
        const std::size_t now = getSecondsSinceEpoch();
        deleteOldQuarantineVersions(docKey, now > MaxAgeSecs ? now - MaxAgeSecs : 0);
        if (isProtected)
            writeState(docKey);

        removeEmptyDocKeys();
        return true;
    }

    LOG_ERR("Quarantining of file [" << docPath << "] to [" << linkedFilePath << "] failed");
    removeEmptyDocKeys();
    return false;
}

bool Quarantine::renewVersion(const std::string& docKey, const std::string& path)
{
    std::lock_guard<std::mutex> lock(Mutex);

    // The version may have gone meanwhile, to make space.
    std::vector<Entry>& fileList = QuarantineMap[docKey];
    if (fileList.empty() || fileList[fileList.size() - 1].fullPath() != path)
        return false;

    const Entry& last = fileList[fileList.size() - 1];
    const uint64_t secondsSinceEpoch = getSecondsSinceEpoch();
    if (secondsSinceEpoch <= last.secondsSinceEpoch())
        return true;

    // The name is the time followed by the rest, which stays.
    const std::string name = Poco::Path(path).getFileName();
    const std::size_t rest = name.find(Delimiter);
    if (rest == std::string::npos)
        return true;

    Entry renewed(QuarantinePath, docKey, secondsSinceEpoch, name.substr(rest), last.size());
    renewed.setProtected(last.isProtected());
    if (::rename(path.c_str(), renewed.fullPath().c_str()) != 0)
    {
        LOG_SYS("Failed to rename quarantined version [" << path << "] to ["
                                                         << renewed.fullPath() << ']');
        return true;
    }

    LOG_DBG("Renewed quarantined version [" << path << "] as [" << renewed.fullPath() << ']');
    fileList[fileList.size() - 1] = renewed;
    if (renewed.isProtected())
        writeState(docKey);

    return true;
}

void Quarantine::removeQuarantinedFiles()
{
    if (!isEnabled())
        return;

    try
    {
        std::lock_guard<std::mutex> lock(Mutex);

        for (const Entry& entry : QuarantineMap[_docKey])
            FileUtil::removeFile(entry.fullPath());
        QuarantineMap.erase(_docKey);
        OpenDocKeys.erase(_docKey);

        const std::string docKeyPath = Poco::Path(QuarantinePath, _docKey).toString();
        FileUtil::removeFile(Poco::Path(docKeyPath, StateFileName).toString());
        if (FileUtil::isEmptyDirectory(docKeyPath))
            FileUtil::removeFile(docKeyPath, /*recursive=*/true);

        LOG_DBG("Removed the quarantined versions of [" << _docKey << ']');
    }
    catch (const std::exception& exc)
    {
        LOG_WRN("Failed to remove the quarantined versions of [" << _docKey
                                                                  << "]: " << exc.what());
    }
}

void Quarantine::markOpen()
{
    if (!isEnabled())
        return;

    try
    {
        std::lock_guard<std::mutex> lock(Mutex);
        OpenDocKeys.insert(_docKey);
        writeState(_docKey);
    }
    catch (const std::exception& exc)
    {
        LOG_WRN("Failed to note [" << _docKey << "] as open in the quarantine: " << exc.what());
    }
}

void Quarantine::markClosed()
{
    if (!isEnabled())
        return;

    try
    {
        std::lock_guard<std::mutex> lock(Mutex);
        OpenDocKeys.erase(_docKey);
        writeState(_docKey);
        removeEmptyDocKeys();
    }
    catch (const std::exception& exc)
    {
        LOG_WRN("Failed to note [" << _docKey << "] as closed in the quarantine: " << exc.what());
    }
}

void Quarantine::protectVersions()
{
    if (!isEnabled())
        return;

    _endedBadly = true;

    try
    {
        std::lock_guard<std::mutex> lock(Mutex);
        for (Entry& entry : QuarantineMap[_docKey])
            entry.setProtected(true);

        // The document stays noted as open until markClosed(), after its last version.
        writeState(_docKey);
        LOG_INF("Protected the quarantined versions of [" << _docKey << ']');
    }
    catch (const std::exception& exc)
    {
        LOG_WRN("Failed to protect the quarantined versions of [" << _docKey
                                                                   << "]: " << exc.what());
    }
}

std::string Quarantine::lastQuarantinedFilePath() const
{
    if (!isEnabled())
        return std::string();

    std::lock_guard<std::mutex> lock(Mutex);

    const auto& fileList = QuarantineMap[_docKey];
    return fileList.empty() ? std::string() : fileList[fileList.size() - 1].fullPath();
}

Quarantine::Entry::Entry(const std::string& root, const std::string& filename)
{
    LOG_TRC("Quarantine file name: [" << filename << ']');

    _fullPath = Poco::Path(root, filename).toString();

    std::vector<StringToken> tokens;
    StringVector::tokenize(filename.c_str(), filename.size(), Delimiter, tokens);
    LOG_TRC("Quarantine file name: [" << filename << "]: " << tokens.size());
    if (tokens.size() > 3)
    {
        _secondsSinceEpoch =
            NumUtil::u64FromString(filename.substr(tokens[0]._index, tokens[0]._length), /*def=*/0);

        _pid =
            NumUtil::u64FromString(filename.substr(tokens[1]._index, tokens[1]._length), /*def=*/0);

        // Note: this is unreliable since both the dockey and filename can (and often do) contain the Delimiter '_'.
        _docKey = filename.substr(tokens[2]._index,
                                  tokens[tokens.size() - 1]._index - tokens[2]._index - 1);

        _filename =
            filename.substr(tokens[tokens.size() - 1]._index, tokens[tokens.size() - 1]._length);

        FileUtil::Stat f(_fullPath);
        _size = f.good() ? f.size() : 0;
    }

    LOG_TRC("Legacy quarantine file for [" << _docKey << "], name: [" << _filename << "], size: "
                                           << _size << ", created: " << _secondsSinceEpoch);
}

Quarantine::Entry::Entry(const std::string& root, const std::string& docKey,
                         const std::string& filename)
{
    _fullPath = Poco::Path(Poco::Path(root, docKey), filename).toString();
    _docKey = docKey;

    std::vector<StringToken> tokens;
    StringVector::tokenize(filename.c_str(), filename.size(), Delimiter, tokens);
    LOG_TRC("Quarantine file for [" << _docKey << "], name: [" << filename
                                    << "]: " << tokens.size());
    if (tokens.size() >= 3)
    {
        _secondsSinceEpoch =
            NumUtil::u64FromString(filename.substr(tokens[0]._index, tokens[0]._length), /*def=*/0);

        _pid =
            NumUtil::u64FromString(filename.substr(tokens[1]._index, tokens[1]._length), /*def=*/0);

        _filename = filename.substr(tokens[2]._index, tokens[2]._length);

        FileUtil::Stat f(_fullPath);
        _size = f.good() ? f.size() : 0;
    }

    LOG_TRC("Quarantine file for [" << _docKey << "], name: [" << _filename << "], size: " << _size
                                    << ", created: " << _secondsSinceEpoch);
}

Quarantine::Entry::Entry(const std::string& root, const std::string& docKey,
                         uint64_t secondsSinceEpoch, const std::string& filename, uint64_t size)
{
    const std::string newFilename = std::to_string(secondsSinceEpoch) + filename;
    _fullPath = Poco::Path(Poco::Path(root, docKey), newFilename).toString();
    _docKey = docKey;

    _secondsSinceEpoch = secondsSinceEpoch;

    _pid = ProcUtil::getProcessId();

    _filename = filename;

    _size = size; // The file isn't quarantined yet, so we use the size of the source.

    LOG_TRC("New quarantine file for [" << _docKey << "], name: [" << _filename << "], size: "
                                        << _size << ", created: " << _secondsSinceEpoch);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
