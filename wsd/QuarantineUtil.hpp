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
 * Document quarantine for suspicious files.
 * Functions: quarantineFile()
 */

#pragma once

#include <common/ContainerUtil.hpp>

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

class DocumentBroker;

class Quarantine
{
    friend class QuarantineWhiteBoxTests;

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

        /// True for a version of a document that ended badly, see protectVersions().
        bool isProtected() const { return _protected; }
        void setProtected(bool isProtected) { _protected = isProtected; }

    private:
        std::string _fullPath; ///< The full path, including the quarantine directory and filename.
        std::string _docKey; ///< The DocKey the file belongs to.
        std::string _filename; ///< The filename, without the path or other components.
        uint64_t _secondsSinceEpoch = 0; ///< The timestamp in the filename.
        uint64_t _size = 0; ///< The size of the file in bytes.
        int _pid = 0; ///< The PID that generated it; informational.
        bool _protected = false; ///< See isProtected().
    };

public:
    Quarantine(DocumentBroker& docBroker, const std::string& docName);

    static void initialize(const std::string& path);

    static bool isEnabled() { return !QuarantinePath.empty(); }

    /// Quarantines a new version of the document. The file is hard-linked when possible, unless
    /// copy is set. Copy a file that the kit may still save over: the engine rewrites a file that
    /// has more than one link in place, which would change the quarantined version too.
    bool quarantineFile(const std::string& docPath, bool copy = false);

    /// Removes every quarantined version of the document, for a document that ended well and
    /// whose versions are not worth keeping.
    void removeQuarantinedFiles();

    /// Notes that the document is open. When wsd goes away before the document is closed, the
    /// versions it has then are protected at the next start, see protectVersions().
    void markOpen();

    /// Notes that the document was closed, after its last version was quarantined.
    void markClosed();

    /// Notes that the document ended badly, with data loss or a crash. Its versions, those it has
    /// now and those quarantined for it from now on, are protected: they are removed to make
    /// space only after all the versions that are not, and never for a new version that is not
    /// protected. The protection is kept over a restart. Call markClosed() after quarantining the
    /// last version.
    void protectVersions();

    /// Returns the last quarantined file's path.
    std::string lastQuarantinedFilePath() const;

private:
    /// Returns quarantine directory size in bytes.
    static std::size_t quarantineSize();

    /// Quarantines a new version of the document; the implementation. A protected version may
    /// take the place of other protected versions.
    static bool quarantineFile(const std::string& docKey, const std::string& docPath,
                               const std::string& quarantinedFilename, bool copy,
                               bool isProtected);

    /// Renames the last version of the document, at path, to the time of now, so it ages from
    /// now. For a version that is quarantined again, unchanged. Returns false when that version is
    /// gone, so the file is to be quarantined as a new version.
    static bool renewVersion(const std::string& docKey, const std::string& path);

    /// Removes expired and excess versions, then more versions until headroomBytes fit within
    /// MaxSizeBytes. The versions that are not protected go first: of the document with the most
    /// of them, the oldest. Then, when mayRemoveProtected is set, the protected ones, oldest
    /// first. When there can't be room, nothing more is removed. Returns true when there is room.
    static bool makeQuarantineSpace(std::size_t headroomBytes, bool mayRemoveProtected);

    /// Forgets the documents that are not open and have no quarantined versions left, and removes
    /// their state files and empty directories.
    static void removeEmptyDocKeys();

    /// Writes the state file of the document: whether it is open, and its protected versions.
    /// A closed document without protected versions has none.
    static void writeState(const std::string& docKey);

    /// Reads the state file of the document under the quarantine directory root at the start, and
    /// protects its versions accordingly.
    static void readState(const std::string& root, const std::string& docKey,
                          std::vector<Entry>& entries);

    /// Removes the versions of the given DocKey that are older than oldestTimestamp, and the
    /// oldest of the ones that are not protected when there are more than MaxVersions of those.
    /// Doesn't remove the DocKey from QuarantineMap if it's empty nor does it remove the DocKey directory.
    static void deleteOldQuarantineVersions(const std::string& docKey, std::size_t oldestTimestamp);

    /// Parses the given Old quarantine-filename into its components, which includes the DocKey.
    static Entry parseOldFilename(const std::string& filename);

    /// Parses the given New quarantine-filename into its components, which doesn't include the DocKey.
    static Entry parseNewFilename(const std::string& filename, const std::string& docKey);

private:
    /// DocKey to Quarantine Entries map.
    static Util::UnorderedStringMap<std::vector<Entry>> QuarantineMap;
    /// The DocKeys of the documents that are open, see markOpen().
    static std::unordered_set<std::string> OpenDocKeys;
    /// Protects the shared QuarantineMap from concurrent modification.
    static std::mutex Mutex;
    static std::string QuarantinePath;
    static std::size_t MaxSizeBytes; ///< Total limit on all quarantined files.
    static std::size_t MaxAgeSecs; ///< The oldest quarantined file for any doc.
    static std::size_t MaxVersions; ///< The maximum number of quarantines per doc.

    /// The delimiter used in the quarantine filename.
    static constexpr char Delimiter = '_';

    /// The file in the directory of a document that holds its state, see writeState().
    static constexpr const char* StateFileName = "state";

    /// The start of the name of a version that is being copied in.
    static constexpr const char* CopyPrefix = "copy-";

    const std::string _docKey;
    const std::string _docName;
    /// The quarantined filename is a multi-part string, formed
    /// from the timestamp, pid, docKey, and document filename.
    /// The Delimiter is used to join and later tokenize it.
    /// The document filename is encoded to ensure tokenization.
    const std::string _quarantinedFilename;
    /// True once the document ended badly, see protectVersions().
    bool _endedBadly = false;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
