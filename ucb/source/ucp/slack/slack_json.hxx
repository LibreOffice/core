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
#include <vector>
#include <sstream>

namespace ucp {
namespace slack {

struct SlackWorkspace {
    rtl::OUString id;
    rtl::OUString name;
    rtl::OUString domain;
    rtl::OUString url;
    bool isActive;

    SlackWorkspace() : isActive(false) {}
};

struct SlackChannel {
    rtl::OUString id;
    rtl::OUString name;
    rtl::OUString purpose;
    rtl::OUString topic;
    bool isPrivate;
    bool isArchived;
    bool isMember;
    sal_Int32 memberCount;

    SlackChannel() : isPrivate(false), isArchived(false), isMember(false), memberCount(0) {}
};

struct SlackUser {
    rtl::OUString id;
    rtl::OUString name;
    rtl::OUString realName;
    rtl::OUString displayName;
    rtl::OUString email;
    bool isBot;
    bool isActive;

    SlackUser() : isBot(false), isActive(false) {}
};

struct SlackUploadInfo {
    rtl::OUString uploadUrl;
    rtl::OUString fileId;
    sal_Int32 expiresIn;

    SlackUploadInfo() : expiresIn(0) {}
};

struct SlackFileInfo {
    rtl::OUString id;
    rtl::OUString name;
    rtl::OUString mimetype;
    sal_Int64 size;
    rtl::OUString url;
    rtl::OUString permalink;
    rtl::OUString timestamp;

    SlackFileInfo() : size(0) {}
};

class SlackJsonHelper
{
public:
    // Parse Slack API responses
    static std::vector<SlackWorkspace> parseWorkspaceList(const rtl::OUString& jsonResponse);
    static std::vector<SlackChannel> parseChannelList(const rtl::OUString& jsonResponse);
    static std::vector<SlackUser> parseUserList(const rtl::OUString& jsonResponse);
    static SlackUploadInfo parseUploadURLResponse(const rtl::OUString& jsonResponse);
    static SlackFileInfo parseCompleteUploadResponse(const rtl::OUString& jsonResponse);

    // Create JSON for Slack API requests
    static rtl::OUString createTokenRequest(const rtl::OUString& authCode);
    static rtl::OUString createUploadURLRequest(const rtl::OUString& filename, sal_Int64 fileSize);
    static rtl::OUString createCompleteUploadRequest(const rtl::OUString& fileId, const rtl::OUString& channelId, const rtl::OUString& message, const rtl::OUString& threadTs);
    static rtl::OUString createChannelListRequest();

    // Parse OAuth2 token response
    static std::pair<rtl::OUString, rtl::OUString> parseTokenResponse(const rtl::OUString& jsonResponse);

    // Error handling
    static bool isErrorResponse(const rtl::OUString& jsonResponse);
    static rtl::OUString extractErrorMessage(const rtl::OUString& jsonResponse);

private:
    static std::string ouStringToStdString(const rtl::OUString& str);
    static rtl::OUString stdStringToOUString(const std::string& str);
};

} // namespace slack
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
