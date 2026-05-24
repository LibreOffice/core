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
 * Document quarantine for suspicious files.
 * Functions: quarantineFile()
 */

#pragma once

#include <common/ContainerUtil.hpp>

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

class DocumentBroker;

class Quarantine
{
    class Entry
    {
    public:
        /// This parses the filename per the original format, which is ambiguous.
        Entry(const std::string& root, const std::string& filename);

        /// This parses the new filename, which is shorter and more reliable.
        Entry(const std::string& root, const std::string& docKey, const std::string& filename);

        /// This creates an entry when quarantining a new file.
        Entry(const std::string& root, const std::string& docKey, uint64_t secondsSinceEpoch,
              const std::string& filename, uint64_t size);

        const std::string& fullPath() const { return _fullPath; }
        uint64_t secondsSinceEpoch() const { return _secondsSinceEpoch; }
        int pid() const { return _pid; }
        const std::string& docKey() const { return _docKey; }
        const std::string& filename() const { return _filename; }
        uint64_t size() const { return _size; }

    private:
        std::string _fullPath; ///< The full path, including the quarantine directory and filename.
        std::string _docKey; ///< The DocKey the file belongs to.
        std::string _filename; ///< The filename, without the path or other components.
        uint64_t _secondsSinceEpoch = 0; ///< The timestamp in the filename.
        uint64_t _size = 0; ///< The size of the file in bytes.
        int _pid = 0; ///< The PID that generated it; informational.
    };

public:
    Quarantine(DocumentBroker& docBroker, const std::string& docName);

    static void initialize(const std::string& path);

    static bool isEnabled() { return !QuarantinePath.empty(); }

    /// Quarantines a new version of the document.
    bool quarantineFile(const std::string& docPath);

    /// Returns the last quarantined file's path.
    std::string lastQuarantinedFilePath() const;

private:
    /// Returns quarantine directory size in bytes.
    static std::size_t quarantineSize();

    /// Quarantines a new version of the document; the implementation.
    static bool quarantineFile(const std::string& docKey, const std::string& docPath,
                               const std::string& quarantinedFilename);

    /// Cleans up quarantined files to make sure we don't exceed MaxSizeBytes.
    static void makeQuarantineSpace(std::size_t headroomBytes);

    /// Removes old quarantined files for the given DocKey based on MaxVersions and @oldestTimestamp.
    /// Doesn't remove the DocKey from QuarantineMap if it's empty nor does it remove the DocKey directory.
    static void deleteOldQuarantineVersions(const std::string& docKey, std::size_t oldestTimestamp);

    /// Parses the given Old quarantine-filename into its components, which includes the DocKey.
    static Entry parseOldFilename(const std::string& filename);

    /// Parses the given New quarantine-filename into its components, which doesn't include the DocKey.
    static Entry parseNewFilename(const std::string& filename, const std::string& docKey);

private:
    /// DocKey to Quarantine Entries map.
    static Util::UnorderedStringMap<std::vector<Entry>> QuarantineMap;
    /// Protects the shared QuarantineMap from concurrent modification.
    static std::mutex Mutex;
    static std::string QuarantinePath;
    static std::size_t MaxSizeBytes; ///< Total limit on all quarantined files.
    static std::size_t MaxAgeSecs; ///< The oldest quarantined file for any doc.
    static std::size_t MaxVersions; ///< The maximum number of quarantines per doc.

    /// The delimiter used in the quarantine filename.
    static constexpr char Delimiter = '_';

    const std::string _docKey;
    const std::string _docName;
    /// The quarantined filename is a multi-part string, formed
    /// from the timestamp, pid, docKey, and document filename.
    /// The Delimiter is used to join and later tokenize it.
    /// The document filename is encoded to ensure tokenization.
    const std::string _quarantinedFilename;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
