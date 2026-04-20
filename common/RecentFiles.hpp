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
#include <string>
#include <vector>
#include <set>

class RecentFiles
{
public:
    // Constructs an empty uninitialised object. Nothing can be done with it before you have called
    // load().
    RecentFiles();

    // Loads the list of recent files and timestamps. The fileName parameter is the file where the
    // list is stored. If it doesn't exist, start with an empty list, but remember the pathname as
    // the list will be saved there whenever a file is added.
    //
    // The maxFiles parameter indicates the maximum number of entries that will be returned from
    // the serialise() member function. At most twice that number of entries are stored in the file,
    // to have a buffer in case some of the files are subsequently removed or temporarily
    // inaccessible when running the app the next time.
    //
    // Only after calling this can you call the other member functions.
    void load(const std::string& fileName, int maxFiles);

    // Add a document URI to the list. It will be placed first in the list. Its timestamp will be
    // the current time. The oldest entry will be dropped if the list had grown larger than the
    // maximum size given when constructing the RecentFiles object. After adding the document, save
    // the list to the same place it was loaded from, or would have been loaded, if it had existed.
    void add(const std::string& uri);

    // Produces a JSON string of the type that our JS expects.
    std::string serialise();

    // As serialise(), but without the documents listed.
    std::string serialiseFiltered(std::set<std::string> dropTheseURIs);

private:
    struct Entry
    {
        std::string uri;
        std::chrono::time_point<std::chrono::system_clock> timestamp;
    };

    bool _initialised;

    std::string _fileName;
    std::vector<Entry> _mostRecentlyUsed;
    int _maxFiles;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
