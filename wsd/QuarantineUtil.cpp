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

#include <Poco/Path.h>
#include <Poco/URI.h>

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
    catch (const std::exception& exc)
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
        catch (const std::exception& exc)
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
        Entry& entry = *entries[0]->begin();
        LOG_DBG("Removing quarantined file [" << entry.filename() << "] for [" << entry.docKey()
                                              << "] with " << entry.size() << " bytes");
        FileUtil::removeFile(entry.fullPath());
        entries[0]->erase(entries[0]->begin());

        currentSize -= entry.size();
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

bool Quarantine::quarantineFile(const std::string& docPath)
{
    try
    {
        return quarantineFile(_docKey, docPath, _quarantinedFilename);
    }
    catch (const std::exception& exc)
    {
        LOG_WRN("Failed to quarantine [" << docPath << "] for docKey [" << _docKey
                                         << "]: " << exc.what());
    }

    return false;
}

bool Quarantine::quarantineFile(const std::string& docKey, const std::string& docPath,
                                const std::string& quarantinedFilename)
{
    if (!isEnabled())
        return false;

    FileUtil::Stat sourceStat(docPath);
    if (!sourceStat.exists())
    {
        LOG_WRN("Quarantining of file [" << docPath << "] failed because it does not exist");
        return false;
    }

    Entry entry(QuarantinePath, docKey, getSecondsSinceEpoch(), quarantinedFilename,
                sourceStat.size());
    Poco::File(Poco::Path(QuarantinePath, docKey)).createDirectories();

    const std::string linkedFilePath = entry.fullPath();
    LOG_TRC("Quarantining [" << docPath << "] to [" << linkedFilePath << ']');

    std::lock_guard<std::mutex> lock(Mutex);

    // Check if we have a duplicate or a new version.
    auto& fileList = QuarantineMap[docKey];
    if (!fileList.empty())
    {
        const std::string lastFile = fileList[fileList.size() - 1].fullPath();
        FileUtil::Stat lastFileStat(lastFile);

        if (FileUtil::Stat::isIdenticalTo(lastFileStat, lastFile, sourceStat, docPath))
        {
            LOG_INF("Quarantining of file ["
                    << docPath << "] to [" << linkedFilePath
                    << "] is skipped because this file version is already quarantined as ["
                    << lastFile << ']');
            return false;
        }
    }

    // Clean-up the quarantine directory and make room for the incoming file.
    makeQuarantineSpace(entry.size());

    if (FileUtil::linkOrCopyFile(docPath, linkedFilePath))
    {
        fileList.emplace_back(entry);
        LOG_INF("Quarantined [" << docPath << "] to [" << linkedFilePath << ']');
        return true;
    }

    LOG_ERR("Quarantining of file [" << docPath << "] to [" << linkedFilePath << "] failed");
    return false;
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
