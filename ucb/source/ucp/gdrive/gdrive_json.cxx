/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gdrive_json.hxx"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <tools/json_writer.hxx>
#include <config_oauth2.h>
#include <com/sun/star/util/DateTime.hpp>

using namespace ucp::gdrive;

std::string GDriveJsonHelper::ouStringToStdString(const rtl::OUString& str)
{
    return rtl::OUStringToOString(str, RTL_TEXTENCODING_UTF8).getStr();
}

rtl::OUString GDriveJsonHelper::stdStringToOUString(const std::string& str)
{
    return rtl::OUString::createFromAscii(str.c_str());
}

std::vector<GDriveFileInfo> GDriveJsonHelper::parseFolderListing(const rtl::OUString& jsonResponse)
{
    std::vector<GDriveFileInfo> aContents;

    if (jsonResponse.isEmpty())
        return aContents;

    try
    {
        boost::property_tree::ptree root;
        std::string jsonStr = ouStringToStdString(jsonResponse);
        std::istringstream jsonStream(jsonStr);
        boost::property_tree::read_json(jsonStream, root);

        // Parse files array
        for (const auto& fileItem : root.get_child("files"))
        {
            const auto& file = fileItem.second;

            GDriveFileInfo info;
            info.id = stdStringToOUString(file.get<std::string>("id", ""));
            info.name = stdStringToOUString(file.get<std::string>("name", ""));
            info.mimeType = stdStringToOUString(file.get<std::string>("mimeType", ""));
            info.size = stdStringToOUString(file.get<std::string>("size", ""));
            info.modifiedTime = stdStringToOUString(file.get<std::string>("modifiedTime", ""));
            info.isFolder = (info.mimeType == "application/vnd.google-apps.folder");

            if (!info.id.isEmpty() && !info.name.isEmpty())
            {
                aContents.push_back(info);
            }
        }
    }
    catch (const boost::property_tree::json_parser_error&)
    {
        // JSON parsing failed
    }
    catch (const boost::property_tree::ptree_bad_path&)
    {
        // Missing "files" field
    }

    return aContents;
}

GDriveFolderListing GDriveJsonHelper::parseFolderListingWithPagination(const rtl::OUString& jsonResponse)
{
    GDriveFolderListing aListing;

    if (jsonResponse.isEmpty())
        return aListing;

    try
    {
        boost::property_tree::ptree root;
        std::string jsonStr = ouStringToStdString(jsonResponse);
        std::istringstream jsonStream(jsonStr);
        boost::property_tree::read_json(jsonStream, root);

        // Parse files array
        auto filesNode = root.get_child_optional("files");
        if (filesNode)
        {
            for (const auto& fileItem : *filesNode)
            {
                const auto& file = fileItem.second;

                GDriveFileInfo info;
                info.id = stdStringToOUString(file.get<std::string>("id", ""));
                info.name = stdStringToOUString(file.get<std::string>("name", ""));
                info.mimeType = stdStringToOUString(file.get<std::string>("mimeType", ""));
                info.size = stdStringToOUString(file.get<std::string>("size", ""));
                info.modifiedTime = stdStringToOUString(file.get<std::string>("modifiedTime", ""));
                info.isFolder = (info.mimeType == "application/vnd.google-apps.folder");

                if (!info.id.isEmpty() && !info.name.isEmpty())
                {
                    aListing.files.push_back(info);
                }
            }
        }

        // Parse pagination token
        std::string nextPageToken = root.get<std::string>("nextPageToken", "");
        aListing.nextPageToken = stdStringToOUString(nextPageToken);
        aListing.hasMore = !nextPageToken.empty();
    }
    catch (const boost::property_tree::json_parser_error&)
    {
        // JSON parsing failed
    }
    catch (const boost::property_tree::ptree_bad_path&)
    {
        // Missing "files" field - not necessarily an error
    }

    return aListing;
}

rtl::OUString GDriveJsonHelper::createFolderMetadata(const rtl::OUString& name, const rtl::OUString& parentId)
{
    tools::JsonWriter writer;
    writer.put("name", ouStringToStdString(name));
    writer.put("mimeType", "application/vnd.google-apps.folder");

    if (!parentId.isEmpty() && parentId != "root")
    {
        auto aParents = writer.startArray("parents");
        writer.putSimpleValue(parentId);  // Use parentId directly as OUString
    }

    return stdStringToOUString(writer.finishAndGetAsOString().getStr());
}

rtl::OUString GDriveJsonHelper::createFileMetadata(const rtl::OUString& name, const rtl::OUString& parentId)
{
    tools::JsonWriter writer;
    writer.put("name", ouStringToStdString(name));

    if (!parentId.isEmpty() && parentId != "root")
    {
        auto aParents = writer.startArray("parents");
        writer.putSimpleValue(parentId);  // Use parentId directly as OUString
    }

    return stdStringToOUString(writer.finishAndGetAsOString().getStr());
}

rtl::OUString GDriveJsonHelper::createCopyMetadata(const rtl::OUString& newName, const rtl::OUString& parentId)
{
    tools::JsonWriter writer;

    if (!newName.isEmpty()) {
        writer.put("name", ouStringToStdString(newName));
    }

    if (!parentId.isEmpty() && parentId != "root") {
        auto aParents = writer.startArray("parents");
        writer.putSimpleValue(parentId);
    }

    return stdStringToOUString(writer.finishAndGetAsOString().getStr());
}

rtl::OUString GDriveJsonHelper::createMoveMetadata(const rtl::OUString& newName, const rtl::OUString& parentId)
{
    tools::JsonWriter writer;

    if (!newName.isEmpty()) {
        writer.put("name", ouStringToStdString(newName));
    }

    if (!parentId.isEmpty() && parentId != "root") {
        auto aParents = writer.startArray("parents");
        writer.putSimpleValue(parentId);
    }

    return stdStringToOUString(writer.finishAndGetAsOString().getStr());
}

rtl::OUString GDriveJsonHelper::createTokenRequest(const rtl::OUString& authCode)
{
    tools::JsonWriter writer;
    writer.put("code", ouStringToStdString(authCode));
    writer.put("client_id", GDRIVE_CLIENT_ID);
    writer.put("client_secret", GDRIVE_CLIENT_SECRET);
    writer.put("redirect_uri", GDRIVE_REDIRECT_URI);
    writer.put("grant_type", "authorization_code");

    return stdStringToOUString(writer.finishAndGetAsOString().getStr());
}

std::pair<rtl::OUString, rtl::OUString> GDriveJsonHelper::parseTokenResponse(const rtl::OUString& jsonResponse)
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

css::util::DateTime GDriveJsonHelper::parseDateTime(const rtl::OUString& dateTimeStr)
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
