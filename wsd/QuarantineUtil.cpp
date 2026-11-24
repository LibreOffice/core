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
#include <mutex>
#include <stdexcept>

namespace
{
std::size_t getSecondsSinceEpoch()
{
    const auto timeNow = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(timeNow.time_since_epoch()).count();
}

} // namespace

std::string Quarantine::QuarantinePath;
Util::UnorderedStringMap<std::vector<Quarantine::Entry>> Quarantine::QuarantineMap;
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
    if (!ConfigUtil::getConfigValue<bool>("quarantine_files[@enable]", false) ||
        !QuarantinePath.empty())
    {
        return;
    }

    MaxSizeBytes =
        ConfigUtil::getConfigValue<std::size_t>("quarantine_files.limit_dir_size_mb", 250) * 1024 *
        1024;
    MaxAgeSecs = ConfigUtil::getConfigValue<std::size_t>("quarantine_files.expiry_min", 3000) * 60;
    MaxVersions = std::max(
        ConfigUtil::getConfigValue<std::size_t>("quarantine_files.max_versions_to_maintain", 5),
        1UL);
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
                    entries.emplace_back(path, docKey, Poco::Path(newFile.path()).getFileName());
                }
            }

            LOG_TRC("Found " << entries.size() << " quarantine file for DocKey [" << docKey << ']');
            if (!entries.empty())
            {
                QuarantineMap[docKey] = std::move(entries);
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

    // Clean up.
    makeQuarantineSpace(/*headroomBytes=*/0);

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

void Quarantine::makeQuarantineSpace(std::size_t headroomBytes)
{
    LOG_ASSERT_MSG(!Mutex.try_lock(), "Quarantine Mutex must be taken");

    if (!isEnabled())
        return;

    std::size_t currentSize = quarantineSize() + headroomBytes;
    if (currentSize > MaxSizeBytes)
    {
        LOG_INF("Quarantine directory is too large at "
                << currentSize - headroomBytes << " bytes for incoming file of " << headroomBytes
                << " bytes (Max Size: " << MaxSizeBytes << " bytes). Shrinking.");
    }

    // We have to remove some files. Start with culling old entries.
    const std::size_t now = getSecondsSinceEpoch();
    for (const auto& pair : QuarantineMap)
    {
        deleteOldQuarantineVersions(pair.first, now - MaxAgeSecs);
    }

    // Check the size again.
    currentSize = quarantineSize() + headroomBytes;
    if (currentSize <= MaxSizeBytes)
    {
        return; // We are good, for now.
    }

    // Ok, we need to remove the oldest entry, but target the document with the most versions first.
    // First, get a reference into all the Entry containers.
    std::vector<std::vector<Entry>*> entries;
    entries.reserve(QuarantineMap.size());
    for (auto& pair : QuarantineMap)
    {
        if (!pair.second.empty())
        {
            entries.emplace_back(&pair.second);
        }
    }

    // Sort by the one with the most entries followed by the one with the older entry.
    std::sort(entries.begin(), entries.end(),
              [](const std::vector<Entry>* const& lhs, const std::vector<Entry>* const& rhs)
              {
                  assert(!lhs->empty() && !rhs->empty() && "Unexpected empty Quarantine Entries");
                  if (lhs->size() != rhs->size())
                  {
                      return lhs->size() > rhs->size();
                  }

                  return (*lhs)[0].secondsSinceEpoch() < (*rhs)[0].secondsSinceEpoch();
              });

    while (!entries.empty())
    {
        // Remove the first entry of the first container.
        assert(!(*entries[0]).empty() && "Unexpected empty Quarantine Entries");
        const Entry& entry = *entries[0]->begin();
        const std::size_t size = entry.size();
        LOG_DBG("Removing quarantined file [" << entry.filename() << "] for [" << entry.docKey()
                                              << "] with " << size << " bytes");
        FileUtil::removeFile(entry.fullPath());
        entries[0]->erase(entries[0]->begin()); // Invalidates entry.

        currentSize -= size;
        if (currentSize <= MaxSizeBytes)
        {
            return; // We are good, for now.
        }

        if (entries[0]->empty())
        {
            entries.erase(entries.begin()); // No quarantines left; remove this container.
            continue; // The next container (if any) should be correctly sorted.
        }

        // Order the top two entries if their sorting has now changed.
        if (entries.size() >= 2)
        {
            const std::vector<Entry>* first = entries[0];
            const std::vector<Entry>* second = entries[1];
            assert(!first->empty() && !second->empty() && "Unexpected empty Quarantine Entries");
            if (second->size() > first->size() ||
                ((second->size() == first->size()) &&
                 (*second)[0].secondsSinceEpoch() < (*first)[0].secondsSinceEpoch()))
            {
                std::swap(entries[0], entries[1]);
            }
        }
    }
}

void Quarantine::deleteOldQuarantineVersions(const std::string& docKey, std::size_t oldestTimestamp)
{
    LOG_ASSERT_MSG(!Mutex.try_lock(), "Quarantine Mutex must be taken");

    if (!isEnabled())
        return;

    auto& container = QuarantineMap[docKey];

    // Check for excessive versions.
    const std::size_t excessVersions =
        container.size() > MaxVersions ? container.size() - MaxVersions : 0;

    // Before removing, check if the remaining ones are too old.
    std::size_t excessAge = 0;
    for (std::size_t i = excessVersions; i < container.size(); ++i)
    {
        // The container is sorted by age, oldest (smallest) first.
        if (oldestTimestamp <= container[i].secondsSinceEpoch())
        {
            break; // These are recent-enough versions.
        }

        excessAge = i;
    }

    if (excessAge > 0)
    {
        LOG_DBG("Removing " << excessAge << " excess quarantined-file versions for [" << docKey
                            << "] from current " << container.size() << " versions, "
                            << excessVersions << " due to exceeding MaxVersions (" << MaxVersions
                            << ") including " << excessAge << " due to being older than "
                            << MaxAgeSecs << " seconds");

        for (std::size_t i = 0; i < excessAge; ++i)
        {
            const std::string& path = container[i].fullPath();
            LOG_TRC("Removing excess quarantined-file version #" << (i + 1) << " [" << path
                                                                 << "] for [" << docKey << ']');

            FileUtil::removeFile(path);
        }

        // And remove them from the container.
        container.erase(container.begin(), container.begin() + excessAge);
    }
}

bool Quarantine::quarantineFile(const std::string& docPath, bool copy)
{
    try
    {
        return quarantineFile(_docKey, docPath, _quarantinedFilename, copy);
    }
    catch (const std::exception& exc)
    {
        LOG_WRN("Failed to quarantine [" << docPath << "] for docKey [" << _docKey
                                         << "]: " << exc.what());
    }

    return false;
}

bool Quarantine::quarantineFile(const std::string& docKey, const std::string& docPath,
                                const std::string& quarantinedFilename, bool copy)
{
    if (!isEnabled())
        return false;

    FileUtil::Stat sourceStat(docPath);
    if (!sourceStat.exists())
    {
        LOG_WRN("Quarantining of file [" << docPath << "] failed because it does not exist");
        return false;
    }

    // A copy larger than the whole quarantine would push out every other version, and then go over
    // the limit anyway.
    if (copy && sourceStat.size() > MaxSizeBytes)
    {
        LOG_WRN("Quarantining of file [" << docPath << "] is skipped because its "
                                         << sourceStat.size()
                                         << " bytes are more than the quarantine can hold");
        return false;
    }

    const std::string docKeyPath = Poco::Path(QuarantinePath, docKey).toString();
    Poco::File(docKeyPath).createDirectories();

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

    // The room for the copy is made once it is there, so a copy that fails costs no version.
    std::string copyPath;
    if (copy)
    {
        copyPath = Poco::Path(docKeyPath, "copy-" + Util::rng::getFilename(12)).toString();
        if (!FileUtil::copy(docPath, copyPath, /*log=*/false, /*throw_on_error=*/false))
        {
            LOG_ERR("Quarantining of file [" << docPath << "] failed to copy it to [" << copyPath
                                             << ']');
            FileUtil::removeFile(copyPath);
            return false;
        }
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
    const std::string linkedFilePath = entry.fullPath();
    LOG_TRC("Quarantining [" << docPath << "] to [" << linkedFilePath << ']');

    // Clean-up the quarantine directory and make room for the incoming file.
    makeQuarantineSpace(entry.size());

    const bool quarantined = copy ? ::rename(copyPath.c_str(), linkedFilePath.c_str()) == 0
                                  : FileUtil::linkOrCopyFile(docPath, linkedFilePath);
    if (quarantined)
    {
        fileList.emplace_back(entry);
        LOG_INF("Quarantined [" << docPath << "] to [" << linkedFilePath << ']');
        return true;
    }

    if (copy)
        FileUtil::removeFile(copyPath);

    LOG_ERR("Quarantining of file [" << docPath << "] to [" << linkedFilePath << "] failed");
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

    const Entry renewed(QuarantinePath, docKey, secondsSinceEpoch, name.substr(rest),
                        last.size());
    if (::rename(path.c_str(), renewed.fullPath().c_str()) != 0)
    {
        LOG_SYS("Failed to rename quarantined version [" << path << "] to ["
                                                         << renewed.fullPath() << ']');
        return true;
    }

    LOG_DBG("Renewed quarantined version [" << path << "] as [" << renewed.fullPath() << ']');
    fileList[fileList.size() - 1] = renewed;
    return true;
}

void Quarantine::removeQuarantinedFiles()
{
    if (!isEnabled())
        return;

    std::lock_guard<std::mutex> lock(Mutex);

    for (const Entry& entry : QuarantineMap[_docKey])
        FileUtil::removeFile(entry.fullPath());
    QuarantineMap.erase(_docKey);

    const std::string docKeyPath = Poco::Path(QuarantinePath, _docKey).toString();
    if (FileUtil::isEmptyDirectory(docKeyPath))
        FileUtil::removeFile(docKeyPath, /*recursive=*/true);

    LOG_DBG("Removed the quarantined versions of [" << _docKey << ']');
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
