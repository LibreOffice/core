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

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

// The templates that the user added, each one a copy of a template file or of a document. The
// folder holds one subfolder per template, named by the id of the template, and the subfolder
// holds the file and, for an ODF file, the preview image from inside it. The name of the template
// is the name of that file without its extension.
class UserTemplates
{
public:
    struct Entry
    {
        // Random hexadecimal digits.
        std::string id;
        std::string name;
        // "writer", "calc", "impress" or "draw".
        std::string type;
        std::string path;
        // A PNG image of the first page, or empty when the file carries none.
        std::string previewPath;
    };

    // Uses the given folder, which is created when the first template is added.
    explicit UserTemplates(std::filesystem::path folder);

    // The templates, sorted by name.
    std::vector<Entry> list() const;

    // Copies the file at sourcePath and names the copy. Returns nothing when the file is neither a
    // template nor a document, the name is empty, or the copy fails.
    std::optional<Entry> addFile(const std::string& sourcePath, const std::string& name);

    std::optional<Entry> rename(const std::string& id, const std::string& name);

    bool remove(const std::string& id);

    // The type of document that a template file creates, from its extension: "writer", "calc",
    // "impress" or "draw". Empty for a file that is not a template.
    static std::string documentTypeOfTemplate(const std::string& fileName);

    // The type of document that a template file or a document file holds, from its extension:
    // "writer", "calc", "impress" or "draw". Empty for a file of any other kind.
    static std::string documentTypeOfFile(const std::string& fileName);

    // The extension of the document that a template file creates, or the extension of a document
    // file itself, in lower case and without the dot. Empty for a file of any other kind.
    static std::string documentExtensionOfFile(const std::string& fileName);

    // The extensions of the template files and the document files that can be added, in lower
    // case and without the dot.
    static std::vector<std::string> acceptedExtensions();

private:
    std::optional<Entry> entryInFolder(const std::filesystem::path& templateFolder) const;

    std::optional<std::filesystem::path> templateFolder(const std::string& id) const;

    std::filesystem::path _folder;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
