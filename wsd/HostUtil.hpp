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

/*
 * Host and alias management utilities.
 * Functions: parseAliases(), isHostAllowed()
 */

#pragma once

#include <config.h>

// HostUtil is only used in non-mobile apps.
#if !MOBILEAPP

#include <common/RegexUtil.hpp>

#include <Poco/URI.h>
#include <Poco/Util/Application.h>

#include <set>
#include <string>

/// This class contains static methods to parse alias_groups and WOPI host and static containers to store the data from the coolwsd.xml
class HostUtil
{
private:
    /// Allowed/denied WOPI hosts, if any and if WOPI is enabled.
    static RegexUtil::RegexListMatcher WopiHosts;
    /// mapping of alias host and port to real host and port
    static std::map<std::string, std::string> AliasHosts;
    /// When group configuration is not defined only the firstHost gets access
    static std::string FirstHost;
    /// list of host (not aliases) in alias_groups
    static std::set<std::string> hostList;
    /// list of allowed websocket origin, used only when indirection_endpoint.geolocation is enabled
    static std::set<std::string> AllowedWSOriginList;

    static bool WopiEnabled;

public:
    /// parse wopi.storage.host
    static void parseWopiHost();

    /// parse wopi.storage.alias_groups.group
    static void parseAliases(Poco::Util::LayeredConfiguration& conf);

    /// parse indirection_endpoint.geolocation_setup.allowed_websocket_origins
    static void parseAllowedWSOrigins();

    /// if request uri is an alias, replace request uri host and port with
    /// original hostname and port defined by group tag from coolwsd.xml
    /// to avoid possibility of opening the same file as two if the WOPI host
    /// is accessed using different aliases
    static std::string getNewUri(const Poco::URI& uri);

    static bool allowedWopiHost(const std::string& host);

    static bool isWopiEnabled() { return WopiEnabled; }

    /// replace the authority of aliashost to realhost if it matches
    static const Poco::URI getNewLockedUri(const Poco::URI& uri);

    static void setFirstHost(const Poco::URI& uri);

    static bool isWopiHostsEmpty();

    static bool allowedWSOrigin(const std::string& origin);

    static bool isForbiddenKitHost(const std::string& host);

private:
    /// parse a single alias from groups of alias_groups, return an empty string if the pattern was invalid
    ///
    /// valid shared.*\.com
    /// valid my-domain-(sub1|sub2).com
    /// valid my-domain.com
    /// invalid my-domain[1
    static std::string parseAlias(const std::string& aliasPattern);

    /// add host to WopiHosts
    static void addWopiHost(const std::string& host, bool allow);

    friend class HostUtilTests;
};
#endif // !MOBILEAPP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
