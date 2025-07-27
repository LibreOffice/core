/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dropbox_json.hxx"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <tools/json_writer.hxx>
#include <config_oauth2.h>
#include <com/sun/star/util/DateTime.hpp>
#include <sal/log.hxx>

using namespace ucp::dropbox;

std::string DropboxJsonHelper::ouStringToStdString(const rtl::OUString& str)
{
    return rtl::OUStringToOString(str, RTL_TEXTENCODING_UTF8).getStr();
}

rtl::OUString DropboxJsonHelper::stdStringToOUString(const std::string& str)
{
    return rtl::OUString::createFromAscii(str.c_str());
}

std::vector<DropboxFileInfo> DropboxJsonHelper::parseFolderListing(const rtl::OUString& jsonResponse)
{
    SAL_WARN("ucb.ucp.dropbox", "parseFolderListing called with response length: " + OUString::number(jsonResponse.getLength()));
    std::vector<DropboxFileInfo> aContents;

    if (jsonResponse.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "JSON response is empty");
        return aContents;
    }

    try
    {
        boost::property_tree::ptree root;
        std::string jsonStr = ouStringToStdString(jsonResponse);
        std::istringstream jsonStream(jsonStr);
        SAL_WARN("ucb.ucp.dropbox", "About to parse JSON with boost");
        boost::property_tree::read_json(jsonStream, root);
        SAL_WARN("ucb.ucp.dropbox", "JSON parsed successfully, looking for entries");

        // Parse entries array (Dropbox API v2 format)
        auto entriesChild = root.get_child_optional("entries");
        if (!entriesChild) {
            SAL_WARN("ucb.ucp.dropbox", "No 'entries' field found in JSON");
            return aContents;
        }
        SAL_WARN("ucb.ucp.dropbox", "Found entries field, iterating through entries");

        for (const auto& entryItem : *entriesChild)
        {
            const auto& entry = entryItem.second;
            SAL_WARN("ucb.ucp.dropbox", "Processing entry...");

            DropboxFileInfo info;
            info.name = stdStringToOUString(entry.get<std::string>("name", ""));

            // Try different ways to get the .tag field
            std::string tagStr = entry.get<std::string>(".tag", "");
            if (tagStr.empty()) {
                // Fallback: try without the dot prefix
                tagStr = entry.get<std::string>("tag", "");
            }
            if (tagStr.empty()) {
                // Default fallback: assume it's a file if name doesn't suggest folder
                tagStr = "file";
            }
            info.tag = stdStringToOUString(tagStr);
            info.isFolder = (info.tag == "folder");
            SAL_WARN("ucb.ucp.dropbox", "Entry name: '" + info.name + "', tag: '" + info.tag + "'");

            // For files, get path_lower as ID and size
            if (info.tag == "file") {
                try {
                    std::string pathLower = entry.get<std::string>("path_lower", "");
                    info.id = stdStringToOUString(pathLower);

                    // If boost parsing failed to get path_lower, try manual extraction
                    if (info.id.isEmpty()) {
                        SAL_WARN("ucb.ucp.dropbox", "path_lower was empty, trying manual extraction");
                        // Manual fallback: use "/" + name
                        info.id = u"/"_ustr + info.name.toAsciiLowerCase();
                        SAL_WARN("ucb.ucp.dropbox", "Manual fallback path: " + info.id);
                    }

                    info.size = stdStringToOUString(std::to_string(entry.get<long>("size", 0)));
                    info.modifiedTime = stdStringToOUString(entry.get<std::string>("client_modified", ""));
                    SAL_WARN("ucb.ucp.dropbox", "Parsed file: " + info.name + " with path: '" + info.id + "'");
                } catch (const std::exception& e) {
                    SAL_WARN("ucb.ucp.dropbox", "Error parsing file data: " + OUString::createFromAscii(e.what()));
                    // Fall back to using name as path if path_lower fails
                    info.id = u"/"_ustr + info.name.toAsciiLowerCase();
                    SAL_WARN("ucb.ucp.dropbox", "Exception fallback path: " + info.id);
                }
            } else if (info.tag == "folder") {
                try {
                    std::string pathLower = entry.get<std::string>("path_lower", "");
                    info.id = stdStringToOUString(pathLower);
                    info.size = u"0"_ustr;
                    info.modifiedTime = rtl::OUString();
                    SAL_WARN("ucb.ucp.dropbox", "Parsed folder: " + info.name + " with path_lower: '" + info.id + "'");
                } catch (const std::exception& e) {
                    SAL_WARN("ucb.ucp.dropbox", "Error parsing folder data: " + OUString::createFromAscii(e.what()));
                    // Fall back to using name as path if path_lower fails
                    info.id = u"/"_ustr + info.name;
                    SAL_WARN("ucb.ucp.dropbox", "Using fallback path: " + info.id);
                }
            }

            if (!info.name.isEmpty())
            {
                aContents.push_back(info);
            }
        }
    }
    catch (const boost::property_tree::json_parser_error& e)
    {
        SAL_WARN("ucb.ucp.dropbox", "JSON parsing failed: " + OUString::createFromAscii(e.what()));
    }
    catch (const boost::property_tree::ptree_bad_path& e)
    {
        SAL_WARN("ucb.ucp.dropbox", "Missing field in JSON: " + OUString::createFromAscii(e.what()));
    }
    catch (const std::exception& e)
    {
        SAL_WARN("ucb.ucp.dropbox", "General exception in JSON parsing: " + OUString::createFromAscii(e.what()));
    }

    return aContents;
}

DropboxFolderListing DropboxJsonHelper::parseFolderListingWithPagination(const rtl::OUString& jsonResponse)
{
    DropboxFolderListing aListing;

    if (jsonResponse.isEmpty())
        return aListing;

    try
    {
        boost::property_tree::ptree root;
        std::string jsonStr = ouStringToStdString(jsonResponse);
        std::istringstream jsonStream(jsonStr);
        boost::property_tree::read_json(jsonStream, root);

        // Parse entries array (Dropbox API v2 format)
        auto entriesNode = root.get_child_optional("entries");
        if (entriesNode)
        {
            for (const auto& entryItem : *entriesNode)
            {
                const auto& entry = entryItem.second;

                DropboxFileInfo info;
                info.name = stdStringToOUString(entry.get<std::string>("name", ""));
                info.tag = stdStringToOUString(entry.get<std::string>(".tag", ""));
                info.isFolder = (info.tag == "folder");

                // For files, get path_lower as ID and size
                if (info.tag == "file") {
                    info.id = stdStringToOUString(entry.get<std::string>("path_lower", ""));
                    info.size = stdStringToOUString(std::to_string(entry.get<long>("size", 0)));
                    info.modifiedTime = stdStringToOUString(entry.get<std::string>("client_modified", ""));
                } else if (info.tag == "folder") {
                    info.id = stdStringToOUString(entry.get<std::string>("path_lower", ""));
                    info.size = u"0"_ustr;
                    info.modifiedTime = rtl::OUString();
                }

                if (!info.name.isEmpty())
                {
                    aListing.files.push_back(info);
                }
            }
        }

        // Parse pagination cursor (Dropbox uses cursor instead of nextPageToken)
        std::string cursor = root.get<std::string>("cursor", "");
        aListing.cursor = stdStringToOUString(cursor);
        aListing.hasMore = root.get<bool>("has_more", false);
    }
    catch (const boost::property_tree::json_parser_error&)
    {
        // JSON parsing failed
    }
    catch (const boost::property_tree::ptree_bad_path&)
    {
        // Missing "entries" field - not necessarily an error
    }

    return aListing;
}

rtl::OUString DropboxJsonHelper::createFolderMetadata(const rtl::OUString& name, const rtl::OUString& parentId)
{
    tools::JsonWriter writer;

    // Dropbox API expects just a path for folder creation
    rtl::OUString path;
    if (parentId.isEmpty() || parentId == "root") {
        path = "/" + name;
    } else {
        path = parentId + "/" + name;
    }

    writer.put("path", ouStringToStdString(path));
    writer.put("autorename", false);

    return stdStringToOUString(writer.finishAndGetAsOString().getStr());
}

rtl::OUString DropboxJsonHelper::createFileMetadata(const rtl::OUString& name, const rtl::OUString& parentId)
{
    tools::JsonWriter writer;

    // Dropbox API expects just a path for file uploads
    rtl::OUString path;
    if (parentId.isEmpty() || parentId == "root") {
        path = "/" + name;
    } else {
        path = parentId + "/" + name;
    }

    writer.put("path", ouStringToStdString(path));
    writer.put("mode", "overwrite");
    writer.put("autorename", false);

    return stdStringToOUString(writer.finishAndGetAsOString().getStr());
}

rtl::OUString DropboxJsonHelper::createCopyMetadata(const rtl::OUString& newName, const rtl::OUString& parentId)
{
    tools::JsonWriter writer;

    // Dropbox API expects from_path and to_path for copy operations
    // This will be filled in by the API client with the source path
    rtl::OUString toPath;
    if (parentId.isEmpty() || parentId == "root") {
        toPath = "/" + newName;
    } else {
        toPath = parentId + "/" + newName;
    }

    writer.put("to_path", ouStringToStdString(toPath));
    writer.put("allow_shared_folder", false);
    writer.put("autorename", false);

    return stdStringToOUString(writer.finishAndGetAsOString().getStr());
}

rtl::OUString DropboxJsonHelper::createMoveMetadata(const rtl::OUString& newName, const rtl::OUString& parentId)
{
    tools::JsonWriter writer;

    // Dropbox API expects from_path and to_path for move operations
    // This will be filled in by the API client with the source path
    rtl::OUString toPath;
    if (parentId.isEmpty() || parentId == "root") {
        toPath = "/" + newName;
    } else {
        toPath = parentId + "/" + newName;
    }

    writer.put("to_path", ouStringToStdString(toPath));
    writer.put("allow_shared_folder", false);
    writer.put("autorename", false);

    return stdStringToOUString(writer.finishAndGetAsOString().getStr());
}

rtl::OUString DropboxJsonHelper::createTokenRequest(const rtl::OUString& authCode)
{
    tools::JsonWriter writer;
    writer.put("code", ouStringToStdString(authCode));
    writer.put("client_id", DROPBOX_CLIENT_ID);
    writer.put("client_secret", DROPBOX_CLIENT_SECRET);
    writer.put("redirect_uri", DROPBOX_REDIRECT_URI);
    writer.put("grant_type", "authorization_code");

    return stdStringToOUString(writer.finishAndGetAsOString().getStr());
}

std::pair<rtl::OUString, rtl::OUString> DropboxJsonHelper::parseTokenResponse(const rtl::OUString& jsonResponse)
{
    rtl::OUString accessToken, refreshToken;

    if (jsonResponse.isEmpty())
        return std::make_pair(accessToken, refreshToken);

    try
    {
        boost::property_tree::ptree root;
        std::string jsonStr = ouStringToStdString(jsonResponse);
        std::istringstream jsonStream(jsonStr);
        boost::property_tree::read_json(jsonStream, root);

        accessToken = stdStringToOUString(root.get<std::string>("access_token", ""));
        refreshToken = stdStringToOUString(root.get<std::string>("refresh_token", ""));
    }
    catch (const boost::property_tree::json_parser_error&)
    {
        // JSON parsing failed
    }

    return std::make_pair(accessToken, refreshToken);
}

css::util::DateTime DropboxJsonHelper::parseDateTime(const rtl::OUString& dateTimeStr)
{
    css::util::DateTime aDateTime;

    if (dateTimeStr.isEmpty())
        return aDateTime;

    // Parse ISO 8601 format: 2024-01-01T12:34:56.789Z
    // Google Drive API returns dates in this format
    std::string dateStr = ouStringToStdString(dateTimeStr);

    try {
        // Extract year, month, day
        if (dateStr.length() >= 10) {
            aDateTime.Year = static_cast<sal_uInt16>(std::stoi(dateStr.substr(0, 4)));
            aDateTime.Month = static_cast<sal_uInt16>(std::stoi(dateStr.substr(5, 2)));
            aDateTime.Day = static_cast<sal_uInt16>(std::stoi(dateStr.substr(8, 2)));
        }

        // Extract hour, minute, second if present
        if (dateStr.length() >= 19) {
            aDateTime.Hours = static_cast<sal_uInt16>(std::stoi(dateStr.substr(11, 2)));
            aDateTime.Minutes = static_cast<sal_uInt16>(std::stoi(dateStr.substr(14, 2)));
            aDateTime.Seconds = static_cast<sal_uInt16>(std::stoi(dateStr.substr(17, 2)));
        }

        // Extract nanoseconds if present
        size_t dotPos = dateStr.find('.');
        if (dotPos != std::string::npos && dotPos + 4 <= dateStr.length()) {
            std::string milliStr = dateStr.substr(dotPos + 1, 3);
            aDateTime.NanoSeconds = static_cast<sal_uInt32>(std::stoi(milliStr)) * 1000000;
        }
    } catch (const std::exception&) {
        // If parsing fails, return empty DateTime
        aDateTime = css::util::DateTime();
    }

    return aDateTime;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
