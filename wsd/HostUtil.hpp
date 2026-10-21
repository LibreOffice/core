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

#include <common/IpNetwork.hpp>

#include <Poco/URI.h>
#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/Util/Application.h>

#include <optional>
#include <regex>
#include <set>
#include <string>
#include <vector>

/// The WOPI hosts allowed or blocked. Each entry is a network in CIDR notation or a regular
/// expression matched against the whole host, ignoring case, for one port or for any port.
class WopiHostList
{
public:
    /// Allow the host on this port, or on any port when the port is 0.
    void allow(const std::string& host, int port);

    /// Block the host on every port.
    void deny(const std::string& host);

    /// Whether the host is allowed on some port, for a check that knows no port.
    bool match(const std::string& host) const { return match(host, AnyPort); }

    /// Whether the host is allowed on this port, by an entry for it or for any port, and is not
    /// blocked.
    bool match(const std::string& host, int port) const;

    void clear() { _entries.clear(); }

    bool empty() const { return _entries.empty(); }

private:
    static constexpr int AnyPort = 0;

    struct Entry
    {
        std::optional<Util::IpNetwork> network;
        std::regex regex;
        int port;
        bool allow;

        bool matches(const std::string& host) const;
    };

    void add(const std::string& host, int port, bool allow);

    std::vector<Entry> _entries;
};

/// This class contains static methods to parse alias_groups and WOPI host and static containers to store the data from the coolwsd.xml
class HostUtil
{
private:
    /// Allowed/denied WOPI hosts, if any and if WOPI is enabled.
    static WopiHostList WopiHosts;
    /// mapping of alias host and port to real host and port
    static std::map<std::string, std::string> AliasHosts;
    /// When group configuration is not defined only the firstHost gets access
    static std::string FirstHost;
    /// True once a host other than FirstHost has been rejected and reported at error level.
    static bool FirstHostRejectionLogged;
    /// list of host (not aliases) in alias_groups
    static std::set<std::string> hostList;

    static bool WopiEnabled;

public:
    /// parse wopi.storage.host
    static void parseWopiHost();

    /// parse wopi.storage.alias_groups.group
    static void parseAliases(Poco::Util::LayeredConfiguration& conf);

    /// if request uri is an alias, replace request uri host and port with
    /// original hostname and port defined by group tag from coolwsd.xml
    /// to avoid possibility of opening the same file as two if the WOPI host
    /// is accessed using different aliases
    static std::string getNewUri(const Poco::URI& uri);

    /// Whether the host is allowed on some port. For a check that knows no port, such as a Host
    /// header or a resolved address on its own.
    [[nodiscard]] static bool allowedWopiHost(const std::string& host);

    /// Whether the host is allowed on this port: by an entry written without a port, or by one
    /// written with this port.
    [[nodiscard]] static bool allowedWopiHost(const std::string& host, int port);

    static bool isWopiEnabled() { return WopiEnabled; }

    /// replace the authority of aliashost to realhost if it matches
    static Poco::URI getNewLockedUri(const Poco::URI& uri);

    static void setFirstHost(const Poco::URI& uri);

    /// True once the host of the first WOPISrc seen has been trusted as the only WOPI host,
    /// because no WOPI host is configured and alias_groups mode is "first".
    static bool isFirstHostTrusted() { return !FirstHost.empty(); }

    static bool isWopiHostsEmpty();

    [[nodiscard]] static bool isForbiddenKitHost(const std::string& host);

private:
    /// parse a single alias from groups of alias_groups, return an empty string if the pattern was invalid
    ///
    /// valid shared.*\.com
    /// valid my-domain-(sub1|sub2).com
    /// valid my-domain.com
    /// invalid my-domain[1
    static std::string parseAlias(const std::string& aliasPattern);

    /// As above, and sets @port to the port written after the host, or to 0 when there is none.
    static std::string parseAlias(const std::string& aliasPattern, int& port);

    /// Parse an alias_groups <host> value into a URI so its host and port can be read. The scheme
    /// is not used for matching; when it is missing https is assumed. Returns std::nullopt when the
    /// value cannot be parsed as a URI, for example an IPv6 literal or a regex in the port.
    static std::optional<Poco::URI> parseHostUri(const std::string& host);

    /// Add a host pattern to the WOPI hosts. A port of 0 means any port.
    static void addWopiHost(const std::string& host, int port, bool allow);

    /// The <host> values after the first one in the alias_groups <group> at @groupPath. Only the
    /// first is used: the others would have to be in groups of their own, or be aliases.
    static std::vector<std::string>
    getExtraGroupHosts(const Poco::Util::AbstractConfiguration& conf,
                       const std::string& groupPath);

    friend class HostUtilTests;
};
#endif // !MOBILEAPP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
