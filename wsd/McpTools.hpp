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
 * Shared input handling for the Model Context Protocol document tools.
 *
 * Every document tool accepts the document in the same set of ways, so the code that turns those
 * arguments into a local file lives here and is reused by each tool.
 */

#pragma once

#include <config.h>

#include <Poco/JSON/Object.h>

#include <cstddef>
#include <istream>
#include <string>

namespace McpTools
{
/// The largest document the endpoint accepts, in bytes, from the ai.mcp.max_file_size setting,
/// which the shipped configuration sets to a finite size. Zero means no limit. It applies to every
/// way a document arrives: inline bytes or an upload.
std::size_t maxDocumentBytes();

/// The largest document a tool call carries inline as base64, in bytes. A larger document is
/// uploaded instead, so the server parses and decodes at most this much in one request.
constexpr std::size_t MaxInlineDocumentBytes = 16 * 1024 * 1024;

/// The largest request body the endpoint parses as one JSON-RPC message, in bytes: an inline
/// document of MaxInlineDocumentBytes as base64, and room for the rest of the call.
constexpr std::size_t MaxMessageBytes = MaxInlineDocumentBytes / 3 * 4 + 1024 * 1024;

/// The longest file name, in bytes, that a document is stored under.
constexpr std::size_t MaxFilenameBytes = 200;

/// The requested name cleaned into a single file-name component, or the neutral default name
/// "document" when the request carries no name to keep. A name that is empty once cleaned, and a
/// name made of nothing but dots, both carry no name. A name longer than MaxFilenameBytes keeps its
/// extension and is cut short before it.
std::string filenameOrDefault(const std::string& requested);

/// A failure reported by the document engine, given as a sentence for the calling model. kind is
/// the engine's failure kind, such as "faileddocloading" or "passwordrequired:to-view", or a short
/// phrase that already says what went wrong. action is the past participle that ends "The document
/// could not be", such as "processed" or "produced". A kind with no sentence of its own is given
/// after that opening.
std::string describeFailure(const std::string& kind, const std::string& action);

/// The result of turning a tool's input arguments into a local document.
struct ResolvedInput
{
    /// True when a document file is ready, false when the input was refused.
    bool ok = false;
    /// When ok is false, a readable message explaining why, meant to be shown to the calling model
    /// as a tool failure.
    std::string errorText;
    /// When ok is true, the local path of the document file.
    std::string fromPath;
    /// The file name to present to the document engine, carrying the extension the engine uses to
    /// tell one document type from another.
    std::string filename;
};

/// Turn the input part of a tool's arguments into a local document file.
///
/// The arguments must set exactly one of "data", the document bytes encoded as base64, or
/// "file_id", an identifier returned by the upload endpoint. An unknown or expired file_id is
/// refused with a message telling the caller to upload again. "filename" names the file; when it is
/// missing the name comes from the upload, or a neutral default is used. The document is written
/// under child-root/tmp/incoming/<random>/, which is copied into the jail with the document.
ResolvedInput resolveInput(const Poco::JSON::Object::Ptr& arguments);

/// Create a fresh random directory under child-root/tmp/incoming, and return the path of a file
/// named after filename inside it, so each document has a directory to itself. The name is always
/// cleaned first so it stays a single component inside that directory. Returns an empty path when
/// the directory could not be created.
std::string makeIncomingPath(const std::string& filename);

/// Write the given bytes to a fresh file named after filename in a new random directory under
/// child-root/tmp/incoming. Returns the path of the written file, or an empty string when the file
/// cannot be written, in which case nothing is left on disk.
std::string writeIncomingDocument(const std::string& filename, const std::string& bytes);

/// Write the next size bytes of source to a fresh file in the same way. Returns an empty string,
/// and leaves nothing on disk, when source ends before size bytes or the file cannot be written.
std::string writeIncomingDocument(const std::string& filename, std::istream& source,
                                  std::size_t size);

/// Give the file at source a second name in a fresh random directory under child-root/tmp/incoming,
/// named after filename, as a hard link or, where that fails, a copy. Returns the new path, or an
/// empty string, leaving nothing on disk, when it cannot be made.
std::string linkIncomingDocument(const std::string& filename, const std::string& source);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
