/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <tools/json_writer.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <vector>
#include <sstream>

namespace ucp {
namespace dropbox {

struct DropboxFileInfo {
    rtl::OUString id;
    rtl::OUString name;
    rtl::OUString tag;  // "file" or "folder" in Dropbox API
    rtl::OUString size;
    rtl::OUString modifiedTime;
    bool isFolder;

    DropboxFileInfo() : isFolder(false) {}
};

struct DropboxFolderListing {
    std::vector<DropboxFileInfo> files;
    rtl::OUString cursor;
    bool hasMore;

    DropboxFolderListing() : hasMore(false) {}
};

class DropboxJsonHelper
{
public:
    // Parse Dropbox API response
    static std::vector<DropboxFileInfo> parseFolderListing(const rtl::OUString& jsonResponse);
    static DropboxFolderListing parseFolderListingWithPagination(const rtl::OUString& jsonResponse);

    // Create JSON for requests
    static rtl::OUString createFolderMetadata(const rtl::OUString& name, const rtl::OUString& parentId);
    static rtl::OUString createFileMetadata(const rtl::OUString& name, const rtl::OUString& parentId);
    static rtl::OUString createCopyMetadata(const rtl::OUString& newName, const rtl::OUString& parentId);
    static rtl::OUString createMoveMetadata(const rtl::OUString& newName, const rtl::OUString& parentId);
    static rtl::OUString createTokenRequest(const rtl::OUString& authCode);

    // Parse token response
    static std::pair<rtl::OUString, rtl::OUString> parseTokenResponse(const rtl::OUString& jsonResponse);

    // Parse ISO 8601 date string from Dropbox API
    static css::util::DateTime parseDateTime(const rtl::OUString& dateTimeStr);

private:
    static std::string ouStringToStdString(const rtl::OUString& str);
    static rtl::OUString stdStringToOUString(const std::string& str);
};

} // namespace dropbox
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */