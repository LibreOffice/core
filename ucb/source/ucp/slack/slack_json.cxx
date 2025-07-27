/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "slack_json.hxx"
#include <config_oauth2.h>
#include <sal/log.hxx>
#include <rtl/ustrbuf.hxx>

namespace ucp {
namespace slack {

std::vector<SlackWorkspace> SlackJsonHelper::parseWorkspaceList(const rtl::OUString& jsonResponse)
{
    std::vector<SlackWorkspace> workspaces;

    try {
        std::istringstream jsonStream(ouStringToStdString(jsonResponse));
        boost::property_tree::ptree pt;
        boost::property_tree::read_json(jsonStream, pt);

        // For now, return single workspace (typical OAuth2 flow)
        SlackWorkspace workspace;
        workspace.id = stdStringToOUString(pt.get<std::string>("team.id", ""));
        workspace.name = stdStringToOUString(pt.get<std::string>("team.name", ""));
        workspace.domain = stdStringToOUString(pt.get<std::string>("team.domain", ""));
        workspace.isActive = true;

        if (!workspace.id.isEmpty()) {
            workspaces.push_back(workspace);
        }

    } catch (const std::exception& e) {
        SAL_WARN("ucb.ucp.slack", "Error parsing workspace list: " << e.what());
    }

    return workspaces;
}

std::vector<SlackChannel> SlackJsonHelper::parseChannelList(const rtl::OUString& jsonResponse)
{
    std::vector<SlackChannel> channels;

    try {
        std::istringstream jsonStream(ouStringToStdString(jsonResponse));
        boost::property_tree::ptree pt;
        boost::property_tree::read_json(jsonStream, pt);

        bool ok = pt.get<bool>("ok", false);
        if (!ok) {
            SAL_WARN("ucb.ucp.slack", "API response not ok");
            return channels;
        }

        // Parse channels array
        for (const auto& item : pt.get_child("channels")) {
            const boost::property_tree::ptree& channelPt = item.second;

            SlackChannel channel;
            channel.id = stdStringToOUString(channelPt.get<std::string>("id", ""));
            channel.name = stdStringToOUString(channelPt.get<std::string>("name", ""));
            channel.purpose = stdStringToOUString(channelPt.get<std::string>("purpose.value", ""));
            channel.topic = stdStringToOUString(channelPt.get<std::string>("topic.value", ""));
            channel.isPrivate = channelPt.get<bool>("is_private", false);
            channel.isArchived = channelPt.get<bool>("is_archived", false);
            channel.isMember = channelPt.get<bool>("is_member", false);
            channel.memberCount = channelPt.get<int>("num_members", 0);

            if (!channel.id.isEmpty() && !channel.isArchived) {
                channels.push_back(channel);
            }
        }

        SAL_WARN("ucb.ucp.slack", "Parsed " << channels.size() << " channels");

    } catch (const std::exception& e) {
        SAL_WARN("ucb.ucp.slack", "Error parsing channel list: " << e.what());
    }

    return channels;
}

std::vector<SlackUser> SlackJsonHelper::parseUserList(const rtl::OUString& jsonResponse)
{
    std::vector<SlackUser> users;

    try {
        std::istringstream jsonStream(ouStringToStdString(jsonResponse));
        boost::property_tree::ptree pt;
        boost::property_tree::read_json(jsonStream, pt);

        // Parse members array
        for (const auto& item : pt.get_child("members")) {
            const boost::property_tree::ptree& userPt = item.second;

            SlackUser user;
            user.id = stdStringToOUString(userPt.get<std::string>("id", ""));
            user.name = stdStringToOUString(userPt.get<std::string>("name", ""));
            user.realName = stdStringToOUString(userPt.get<std::string>("real_name", ""));
            user.displayName = stdStringToOUString(userPt.get<std::string>("profile.display_name", ""));
            user.email = stdStringToOUString(userPt.get<std::string>("profile.email", ""));
            user.isBot = userPt.get<bool>("is_bot", false);
            user.isActive = !userPt.get<bool>("deleted", false);

            if (!user.id.isEmpty() && user.isActive && !user.isBot) {
                users.push_back(user);
            }
        }

    } catch (const std::exception& e) {
        SAL_WARN("ucb.ucp.slack", "Error parsing user list: " << e.what());
    }

    return users;
}

SlackUploadInfo SlackJsonHelper::parseUploadURLResponse(const rtl::OUString& jsonResponse)
{
    SlackUploadInfo uploadInfo;

    try {
        std::istringstream jsonStream(ouStringToStdString(jsonResponse));
        boost::property_tree::ptree pt;
        boost::property_tree::read_json(jsonStream, pt);

        bool ok = pt.get<bool>("ok", false);
        if (ok) {
            uploadInfo.uploadUrl = stdStringToOUString(pt.get<std::string>("upload_url", ""));
            uploadInfo.fileId = stdStringToOUString(pt.get<std::string>("file_id", ""));
            uploadInfo.expiresIn = pt.get<int>("expires_in", 0);
        }

    } catch (const std::exception& e) {
        SAL_WARN("ucb.ucp.slack", "Error parsing upload URL response: " << e.what());
    }

    return uploadInfo;
}

SlackFileInfo SlackJsonHelper::parseCompleteUploadResponse(const rtl::OUString& jsonResponse)
{
    SlackFileInfo fileInfo;

    try {
        std::istringstream jsonStream(ouStringToStdString(jsonResponse));
        boost::property_tree::ptree pt;
        boost::property_tree::read_json(jsonStream, pt);

        bool ok = pt.get<bool>("ok", false);
        if (ok) {
            const boost::property_tree::ptree& filePt = pt.get_child("file");
            fileInfo.id = stdStringToOUString(filePt.get<std::string>("id", ""));
            fileInfo.name = stdStringToOUString(filePt.get<std::string>("name", ""));
            fileInfo.mimetype = stdStringToOUString(filePt.get<std::string>("mimetype", ""));
            fileInfo.size = filePt.get<long long>("size", 0);
            fileInfo.url = stdStringToOUString(filePt.get<std::string>("url_private", ""));
            fileInfo.permalink = stdStringToOUString(filePt.get<std::string>("permalink", ""));
            fileInfo.timestamp = stdStringToOUString(filePt.get<std::string>("timestamp", ""));
        }

    } catch (const std::exception& e) {
        SAL_WARN("ucb.ucp.slack", "Error parsing complete upload response: " << e.what());
    }

    return fileInfo;
}

rtl::OUString SlackJsonHelper::createTokenRequest(const rtl::OUString& authCode)
{
    rtl::OUStringBuffer request;
    request.append("client_id=");
    request.append(SLACK_CLIENT_ID);
    request.append("&client_secret=");
    request.append(SLACK_CLIENT_SECRET);
    request.append("&code=");
    request.append(authCode);
    request.append("&redirect_uri=");
    request.append(SLACK_REDIRECT_URI);

    return request.makeStringAndClear();
}

rtl::OUString SlackJsonHelper::createUploadURLRequest(const rtl::OUString& filename, sal_Int64 fileSize)
{
    tools::JsonWriter aJson;
    aJson.put("filename", filename);
    aJson.put("length", fileSize);

    return rtl::OUString::fromUtf8(aJson.finishAndGetAsOString());
}

rtl::OUString SlackJsonHelper::createCompleteUploadRequest(const rtl::OUString& fileId, const rtl::OUString& channelId, const rtl::OUString& message, const rtl::OUString& threadTs)
{
    tools::JsonWriter aJson;

    // Create files array with single file
    aJson.startArray("files");
    aJson.startObject();
    aJson.put("id", fileId);
    aJson.put("title", message.isEmpty() ? rtl::OUString("Document") : message);
    aJson.endObject();
    aJson.endArray();

    // Set channel
    aJson.put("channel_id", channelId);

    // Add initial comment if provided
    if (!message.isEmpty()) {
        aJson.put("initial_comment", message);
    }

    // Add thread timestamp if replying to thread
    if (!threadTs.isEmpty()) {
        aJson.put("thread_ts", threadTs);
    }

    return rtl::OUString::fromUtf8(aJson.finishAndGetAsOString());
}

rtl::OUString SlackJsonHelper::createChannelListRequest()
{
    // For GET request, no body needed
    return rtl::OUString();
}

std::pair<rtl::OUString, rtl::OUString> SlackJsonHelper::parseTokenResponse(const rtl::OUString& jsonResponse)
{
    std::pair<rtl::OUString, rtl::OUString> tokens;

    try {
        std::istringstream jsonStream(ouStringToStdString(jsonResponse));
        boost::property_tree::ptree pt;
        boost::property_tree::read_json(jsonStream, pt);

        bool ok = pt.get<bool>("ok", false);
        if (ok) {
            tokens.first = stdStringToOUString(pt.get<std::string>("access_token", ""));
            // Slack doesn't typically provide refresh tokens
            tokens.second = rtl::OUString();
        } else {
            SAL_WARN("ucb.ucp.slack", "Token response not ok: " << jsonResponse);
        }

    } catch (const std::exception& e) {
        SAL_WARN("ucb.ucp.slack", "Error parsing token response: " << e.what());
    }

    return tokens;
}

bool SlackJsonHelper::isErrorResponse(const rtl::OUString& jsonResponse)
{
    try {
        std::istringstream jsonStream(ouStringToStdString(jsonResponse));
        boost::property_tree::ptree pt;
        boost::property_tree::read_json(jsonStream, pt);

        return !pt.get<bool>("ok", false);

    } catch (const std::exception&) {
        return true; // Invalid JSON is considered an error
    }
}

rtl::OUString SlackJsonHelper::extractErrorMessage(const rtl::OUString& jsonResponse)
{
    try {
        std::istringstream jsonStream(ouStringToStdString(jsonResponse));
        boost::property_tree::ptree pt;
        boost::property_tree::read_json(jsonStream, pt);

        return stdStringToOUString(pt.get<std::string>("error", "Unknown error"));

    } catch (const std::exception&) {
        return "Invalid JSON response";
    }
}

std::string SlackJsonHelper::ouStringToStdString(const rtl::OUString& str)
{
    return std::string(str.toUtf8().getStr());
}

rtl::OUString SlackJsonHelper::stdStringToOUString(const std::string& str)
{
    return rtl::OUString::fromUtf8(str.c_str());
}

} // namespace slack
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
