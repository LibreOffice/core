/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2009-2010 Griffin Brown Digital Publishing Ltd.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OFFICEOTRON_ZIPFILE_HXX
#define OFFICEOTRON_ZIPFILE_HXX

#include <memory>
#include <string>
#include <vector>

// One entry of the zip central directory.
struct ZipEntryInfo
{
    std::string name;

    // Bit 3 of the general purpose flags: the sizes were written in a
    // data descriptor after the entry data.
    bool dataDescriptor = false;

    bool isDirectory() const { return !name.empty() && name.back() == '/'; }
};

// Read access to a zip archive. The central directory parsing, the
// zip64 handling and the entry decompression come from the engine's
// package module (the ZipFile class of the package2 library), so the
// validator reads packages with the same code the engine itself uses.
class ZipArchive
{
public:
    ZipArchive();
    ~ZipArchive();

    ZipArchive(const ZipArchive&) = delete;
    ZipArchive& operator=(const ZipArchive&) = delete;

    bool open(const std::string& path, std::string& error);

    const std::vector<ZipEntryInfo>& entries() const;

    // Returns null when no entry of that exact name exists.
    const ZipEntryInfo* find(const std::string& name) const;

    bool extract(const ZipEntryInfo& entry, std::string& out, std::string& error) const;

    // Convenience overload: returns the entry, or null when it is absent
    // or fails to extract.
    const ZipEntryInfo* extract(const std::string& name, std::string& out) const;

    size_t centralRecordCount() const;

    // The number of central directory entries whose recorded offset holds
    // a valid local header signature. A value below centralRecordCount
    // means the two structures disagree.
    size_t localHeaderCount() const;

    bool usesDataDescriptors() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
