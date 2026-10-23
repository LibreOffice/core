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

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

/// Local document version store for the desktop/mobile apps: timestamped
/// copies of documents in an app-private directory, kept because autosave
/// overwrites the user's file in place.
///
/// The public API is STL-only (see common/Prefs.hpp for the precedent) so
/// CODA-W/CODA-M can reuse it as-is; all entry points are no-ops until
/// initialize() is called by the app.
///
/// Documents are keyed by FileUtil::hostPath(). A rename or move outside
/// the app starts a new history. The directory scan is the source of
/// truth; meta.json is only a cache.
namespace versioning
{

struct Limits
{
    /// When false, snapshots are no-ops but existing history remains
    /// browsable and is still pruned at initialize().
    bool enabled = true;
    /// Snapshots (other than bypassing ones) are skipped while the newest
    /// stored version is younger than this.
    std::chrono::seconds mergeWindow{ 600 };
    std::size_t maxVersionsPerDoc = 120;
    std::uint64_t maxFileSizeBytes = 128ULL * 1024 * 1024;
    /// Global store cap; enforced by evicting oldest versions from the
    /// documents holding the most versions.
    std::uint64_t maxStoreSizeBytes = 4096ULL * 1024 * 1024;
};

struct VersionInfo
{
    std::string fileName;
    /// Parsed from the version filename.
    std::chrono::system_clock::time_point timestamp;
    std::uint64_t size = 0;
};

enum class SnapshotResult : std::uint8_t
{
    Created,
    Deduplicated, ///< Content identical to the newest stored version.
    Coalesced, ///< Newest stored version is younger than the merge window.
    TooLarge, ///< Document exceeds Limits::maxFileSizeBytes or Limits::maxStoreSizeBytes.
    Excluded, ///< Path is under an excluded prefix.
    Disabled, ///< Store not initialized or snapshots disabled.
    Failed ///< IO failure; logged, never fatal.
};

/// Root the store at @rootPath (created 0700 on demand), rebuild the
/// index from a directory scan, sweep orphan temporaries and prune.
/// Documents whose path starts with any of @excludedPathPrefixes are
/// never versioned; exportVersion() copies into @exportDirectory.
void initialize(const std::string& rootPath, const Limits& limits,
                std::vector<std::string> excludedPathPrefixes,
                const std::string& exportDirectory);

/// True when initialized and snapshots are enabled.
bool isEnabled();

/// The limits currently in force (the defaults until initialize()).
Limits currentLimits();

/// Replace the limits and prune to fit them immediately.
void setLimits(const Limits& limits);

/// Queue an asynchronous snapshot of @docPath. Cheap no-op when disabled;
/// a request while a snapshot of the same path is in flight runs once
/// that one ends. Failures are logged, never surfaced.
void snapshot(std::string docPath, bool bypassMergeWindow);

/// Copies outside the store lock: a version of the document that is
/// deleted during the copy does not stop this one.
SnapshotResult snapshotSync(const std::string& docPath, bool bypassMergeWindow);

/// @hook runs in snapshotSync() once the copy is on disk, before the store
/// takes it. It must not throw.
void setCopyHookForTest(std::function<void()> hook);

/// Stored versions of @docPath, newest first.
std::vector<VersionInfo> listVersions(const std::string& docPath);

/// Copy version @versionFileName of @docPath into the export directory,
/// named after the document and the version timestamp. Returns the
/// exported path, or empty on failure.
std::string exportVersion(const std::string& docPath, const std::string& versionFileName);

void clearHistory(const std::string& docPath);

/// Delete every stored version of every document.
void clearAll();

std::uint64_t storeSizeBytes();

/// Apply retention to every document and enforce the global cap.
void pruneNow(std::chrono::system_clock::time_point now);

/// The store directory a document's versions live in (whether or not it
/// exists yet). Empty until initialized.
std::string docDirectory(const std::string& docPath);

/// Format @tp as a version-filename timestamp ("%Y%m%d-%H%M%S", UTC).
std::string formatTimestamp(std::chrono::system_clock::time_point tp);

/// Parse a version filename ("<timestamp>[-N].<ext>"). Returns false for
/// anything that does not match the grammar exactly; such files are
/// treated as orphan temporaries by the startup sweep.
bool parseVersionFilename(const std::string& fileName,
                          std::chrono::system_clock::time_point& timestamp);

/// Pure retention policy.
namespace policy
{
/// Return the indices into @newestFirst that retention deletes: tiered
/// thinning by age, then a smallest-gap cull down to @maxCount; the
/// newest few versions are never deleted.
std::vector<std::size_t> thin(const std::vector<VersionInfo>& newestFirst,
                              std::chrono::system_clock::time_point now, std::size_t maxCount);
} // namespace policy

} // namespace versioning

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
