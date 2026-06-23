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

#include "config.h"

#include <fstream>
#include <string>

#include "Poco/Exception.h"
#include "Poco/Path.h"
#include "Poco/Types.h"
#include "Poco/Zip/Decompress.h"
#include "Poco/Zip/ZipArchive.h"
#include "Poco/Zip/ZipFileInfo.h"

#include "common/Log.hpp"

#include "Unzip.hpp"

namespace {
// Reject absolute paths and ".." segments so an entry cannot escape the destination dir:
bool isSafeEntryName(std::string const & name) {
    if (name.empty()) {
        return false;
    }
    if (name.starts_with("/")) {
        return false;
    }
    if (name == ".." || name.starts_with("../") || name.ends_with("/..")
        || name.find("/../") != std::string::npos)
    {
        return false;
    }
    // Poco's internals don't seem to guard against NUL chars when decaying from std::string to char
    // pointer:
    if (name.find('\0') != std::string::npos) {
        return false;
    }
    return true;
}
}

bool Unzip::extract(std::string const & zipPath, std::string const & destDir) {
    std::ifstream in(zipPath, std::ios::binary);
    if (!in) {
        LOG_ERR("unzip: cannot open [" << zipPath << ']');
        return false;
    }
    try {
        // Pre-scan the central directory for issues:
        Poco::Zip::ZipArchive const archive(in);
        constexpr Poco::UInt64 maxUncompressedSize = 256ULL * 1024 * 1024;
            // safety guard against patch bombs
        Poco::UInt64 total = 0;
        for (auto it = archive.fileInfoBegin(); it != archive.fileInfoEnd(); ++it) {
            if (!isSafeEntryName(it->first)) {
                LOG_ERR("unzip: unsafe entry name in [" << zipPath << "]: [" << it->first << ']');
                return false;
            }
            auto const entrySize = it->second.getUncompressedSize();
            if (entrySize > maxUncompressedSize - total) {
                LOG_ERR(
                    "unzip: [" << zipPath << "] uncompressed size exceeds the "
                    << maxUncompressedSize << "-byte cap");
                return false;
            }
            total += entrySize;
        }
        in.clear();
        in.seekg(0);
        Poco::Zip::Decompress decompress(in, Poco::Path(destDir));
        decompress.decompressAllFiles();
    } catch (Poco::Exception const & e) {
        LOG_ERR("unzip: [" << zipPath << "]: " << e.displayText());
        return false;
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
