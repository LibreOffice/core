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

/// Drafts: documents the app created that have no place the user chose yet. They live in an
/// app-private folder until a Save As moves them out.
///
/// The teardown save of a closing document can write a discarded draft again, so deletion is
/// deferred to the next start: a deletion mark is an empty file of the same name under
/// <drafts>/.marked.
///
/// Paths are UTF-8. initialize() comes before any other call.
namespace drafts
{
/// Use @appDataDir/drafts, creating it. Returns false when it cannot be created.
bool initialize(const std::string& appDataDir);

const std::string& directory();

/// The path of a draft "<stem>.<extension>", or "<stem> (<n>).<extension>" with the lowest n
/// from 1, whose name is neither taken by a file nor marked for deletion. Empty when the name
/// is not a plain file name.
std::string nextFreeName(const std::string& stem, const std::string& extension);

/// Copy @sourcePath to nextFreeName(@stem, @extension). Never replaces a file; a name taken
/// meanwhile moves on to the next one. Returns the new path, or empty on failure.
std::string createFromTemplate(const std::string& sourcePath, const std::string& stem,
                               const std::string& extension);

/// Marks are by file name: the folder part of @path is ignored.
void markForDeletion(const std::string& path);
void unmarkForDeletion(const std::string& path);
bool isMarkedForDeletion(const std::string& path);

/// Mark, then remove; if the app dies in between, the next start still deletes it. Returns
/// whether the file is gone.
bool discard(const std::string& path);

/// A mark stays while its draft cannot be deleted.
void deleteMarked();

/// Whether @path, which need not exist, is inside the drafts folder, comparing host paths.
bool contains(const std::string& path);
} // namespace drafts

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
