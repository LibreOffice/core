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

#include <common/UserTemplates.hpp>

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

// The files that the server of one cloud account lists in its settings lists, kept on disk. The
// "shared" list holds the files of the whole server and the "user" list the files of the user.
// Each file has its own folder, presets/<list>/<group>/<file name>/ in the folder of the account,
// which holds the file, the stamp the list gave for it in the file "stamp", and the preview image
// when the file carries one. Only the group "template" is stored.
class AccountPresets
{
public:
    // One entry of a group of a settings list.
    struct Listed
    {
        std::string uri;
        // Changes when the content of the file changes.
        std::string stamp;
    };

    // The size of the largest file that is stored.
    static constexpr std::size_t MaxFileBytes = 50 * 1024 * 1024;

    // The length of the longest stamp that is stored.
    static constexpr std::size_t MaxStampBytes = 256;

    // The largest number of files that one group of one list holds.
    static constexpr std::size_t MaxFilesPerGroup = 100;

    explicit AccountPresets(std::filesystem::path accountFolder);

    // The file name a settings uri gives: its file_name query parameter, or else the last path
    // segment, decoded. Nothing when that is not one plain file name.
    static std::optional<std::string> fileNameOfUri(const std::string& uri);

    // Removes the stored files of this list and group that listed does not name.
    void removeUnlisted(const std::string& list, const std::string& group,
                        const std::vector<Listed>& listed);

    // The uris of listed whose file is missing or has another stamp, and that storeFile can store.
    std::vector<std::string> outdatedUris(const std::string& list, const std::string& group,
                                          const std::vector<Listed>& listed) const;

    // Stores bytes as the file that uri names, with its stamp: one part at offset 0, and then the
    // finish. False when storeFilePart or finishFile gives false.
    bool storeFile(const std::string& list, const std::string& group, const std::string& uri,
                   const std::string& stamp, const std::string& bytes);

    // Stores bytes as the part at offset of the file that uri names. The parts of a file come in
    // order: a part at offset 0 starts the file, and each next part starts where the parts before
    // it end. The file is stored when finishFile gives it its stamp. False when list is not
    // "shared" or "user", group is not "template", the uri does not give the plain name of a
    // template or document file, offset is not where the next part starts, or the parts are
    // together larger than MaxFileBytes. Also false when the file is new and the group of the
    // list already holds MaxFilesPerGroup.
    bool storeFilePart(const std::string& list, const std::string& group, const std::string& uri,
                       std::size_t offset, const std::string& bytes);

    // Stores the parts of the file that uri names as that file, with its stamp. False when the
    // file has no parts or stamp is longer than MaxStampBytes, and the parts are then removed.
    bool finishFile(const std::string& list, const std::string& group, const std::string& uri,
                    const std::string& stamp);

    // The stored templates, shared before user, each sorted by name. The id is
    // "<list>/<file name>".
    std::vector<UserTemplates::Entry> templates() const;

    // Removes every stored file of the account.
    void removeAll();

private:
    // The folder of one group of one list, or nothing when the list or the group is not stored.
    std::optional<std::filesystem::path> groupFolder(const std::string& list,
                                                     const std::string& group) const;

    std::filesystem::path _presetsFolder;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
