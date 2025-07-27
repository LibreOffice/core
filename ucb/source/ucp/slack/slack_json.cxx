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
        SAL_WARN("ucb.ucp.slack", "Starting to parse channels array");

        // Check if 'channels' key exists
        auto channelsIter = pt.find("channels");
        if (channelsIter == pt.not_found()) {
            SAL_WARN("ucb.ucp.slack", "'channels' key not found in response");

            // Try 'conversations' instead (alternative key name)
            channelsIter = pt.find("conversations");
            if (channelsIter == pt.not_found()) {
                SAL_WARN("ucb.ucp.slack", "'conversations' key also not found in response");
                return channels;
            } else {
                SAL_WARN("ucb.ucp.slack", "Found 'conversations' key instead of 'channels'");
            }
        }

        const auto& channelsChild = channelsIter->second;
        SAL_WARN("ucb.ucp.slack", "Found channels array with " << channelsChild.size() << " items");

        for (const auto& item : channelsChild) {
            const boost::property_tree::ptree& channelPt = item.second;

            SlackChannel channel;
            channel.id = stdStringToOUString(channelPt.get<std::string>("id", ""));

            // Get channel type to handle DMs vs channels differently
            std::string channelType = channelPt.get<std::string>("is_channel", "false") == "true" ? "channel" :
                                     channelPt.get<std::string>("is_group", "false") == "true" ? "group" :
                                     channelPt.get<std::string>("is_im", "false") == "true" ? "im" :
                                     channelPt.get<std::string>("is_mpim", "false") == "true" ? "mpim" : "unknown";

            // Handle different naming for DMs vs channels
            if (channelType == "im") {
                // For DMs, use user ID or "Direct Message" as fallback
                std::string userName = channelPt.get<std::string>("user", "");
                if (!userName.empty()) {
                    channel.name = stdStringToOUString("@" + userName);
                } else {
                    channel.name = u"Direct Message"_ustr;
                }
                channel.isPrivate = true;
            } else if (channelType == "mpim") {
                // For group DMs, try to get a name or use fallback
                std::string name = channelPt.get<std::string>("name", "");
                if (!name.empty()) {
                    channel.name = stdStringToOUString(name);
                } else {
                    channel.name = u"Group Message"_ustr;
                }
                channel.isPrivate = true;
            } else {
                // Regular channels
                channel.name = stdStringToOUString(channelPt.get<std::string>("name", ""));
                channel.isPrivate = channelPt.get<bool>("is_private", false);
            }

            channel.purpose = stdStringToOUString(channelPt.get<std::string>("purpose.value", ""));
            channel.topic = stdStringToOUString(channelPt.get<std::string>("topic.value", ""));
            channel.isArchived = channelPt.get<bool>("is_archived", false);
            channel.isMember = channelPt.get<bool>("is_member", true); // DMs are always "member"
            channel.memberCount = channelPt.get<int>("num_members", 0);

            // Include if not archived and has valid ID
            SAL_WARN("ucb.ucp.slack", "Processing channel: id=" << channel.id
                                    << ", name=" << channel.name
                                    << ", archived=" << channel.isArchived
                                    << ", type=" << channelType);

            if (!channel.id.isEmpty() && !channel.isArchived) {
                channels.push_back(channel);
                SAL_WARN("ucb.ucp.slack", "Channel added to list");
            } else {
                SAL_WARN("ucb.ucp.slack", "Channel rejected: "
                                        << (channel.id.isEmpty() ? "empty ID" : "")
                                        << (channel.isArchived ? " archived" : ""));
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
        SAL_WARN("ucb.ucp.slack", "Complete upload response: " << jsonResponse);

        std::istringstream jsonStream(ouStringToStdString(jsonResponse));
        boost::property_tree::ptree pt;
        boost::property_tree::read_json(jsonStream, pt);

        bool ok = pt.get<bool>("ok", false);
        SAL_WARN("ucb.ucp.slack", "Response ok: " << (ok ? "true" : "false"));

        if (ok) {
            // Try to get files array first (files.completeUploadExternal returns an array)
            auto filesOpt = pt.get_child_optional("files");
            if (filesOpt && !filesOpt->empty()) {
                // Get first file from the files array
                const boost::property_tree::ptree& filePt = filesOpt->begin()->second;
                fileInfo.id = stdStringToOUString(filePt.get<std::string>("id", ""));
                fileInfo.name = stdStringToOUString(filePt.get<std::string>("name", ""));
                fileInfo.mimetype = stdStringToOUString(filePt.get<std::string>("mimetype", ""));
                fileInfo.size = filePt.get<long long>("size", 0);
                fileInfo.url = stdStringToOUString(filePt.get<std::string>("url_private", ""));
                fileInfo.permalink = stdStringToOUString(filePt.get<std::string>("permalink", ""));
                fileInfo.timestamp = stdStringToOUString(filePt.get<std::string>("timestamp", ""));
                SAL_WARN("ucb.ucp.slack", "Parsed file info: id=" << fileInfo.id << " name=" << fileInfo.name);
            } else {
                // Fallback: try single file object (for other API endpoints)
                auto fileOpt = pt.get_child_optional("file");
                if (fileOpt) {
                    const boost::property_tree::ptree& filePt = *fileOpt;
                    fileInfo.id = stdStringToOUString(filePt.get<std::string>("id", ""));
                    fileInfo.name = stdStringToOUString(filePt.get<std::string>("name", ""));
                    fileInfo.mimetype = stdStringToOUString(filePt.get<std::string>("mimetype", ""));
                    fileInfo.size = filePt.get<long long>("size", 0);
                    fileInfo.url = stdStringToOUString(filePt.get<std::string>("url_private", ""));
                    fileInfo.permalink = stdStringToOUString(filePt.get<std::string>("permalink", ""));
                    fileInfo.timestamp = stdStringToOUString(filePt.get<std::string>("timestamp", ""));
                    SAL_WARN("ucb.ucp.slack", "Parsed file info: id=" << fileInfo.id << " name=" << fileInfo.name);
                } else {
                    SAL_WARN("ucb.ucp.slack", "No 'files' array or 'file' object found in response");
                }
            }
        } else {
            SAL_WARN("ucb.ucp.slack", "Response indicates failure");
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
    // Slack files.getUploadURLExternal expects form data, not JSON
    rtl::OUStringBuffer request;
    request.append("filename=");
    request.append(filename);
    request.append("&length=");
    request.append(rtl::OUString::number(fileSize));

    return request.makeStringAndClear();
}

rtl::OUString SlackJsonHelper::createCompleteUploadRequest(const rtl::OUString& fileId, const rtl::OUString& channelId, const rtl::OUString& message, const rtl::OUString& threadTs)
{
    // Slack files.completeUploadExternal expects form data with files as JSON array
    rtl::OUStringBuffer request;

    // Create the files JSON array manually (tools::JsonWriter creates an object, but we need just an array)
    rtl::OUStringBuffer filesJson;
    filesJson.append("[{\"id\":\"");
    filesJson.append(fileId);
    filesJson.append("\",\"title\":\"");
    filesJson.append(message.isEmpty() ? rtl::OUString("Document") : message);
    filesJson.append("\"}]");

    request.append("files=");
    request.append(filesJson.toString());
    request.append("&channel_id=");
    request.append(channelId);

    // Add initial comment if provided
    if (!message.isEmpty()) {
        request.append("&initial_comment=");
        request.append(message);
    }

    // Add thread timestamp if replying to thread
    if (!threadTs.isEmpty()) {
        request.append("&thread_ts=");
        request.append(threadTs);
    }

    return request.makeStringAndClear();
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
