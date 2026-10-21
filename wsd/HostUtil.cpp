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
 * Implementation of host validation and alias parsing.
 * Functions: parseAliases(), isHostAllowed()
 */

#include <config.h>

// HostUtil is only used in non-mobile apps.
#if !MOBILEAPP

#include "HostUtil.hpp"

#include <common/CommandControl.hpp>
#include <common/ConfigUtil.hpp>
#include <common/IpNetwork.hpp>
#include <common/Log.hpp>
#include <common/RegexUtil.hpp>
#include <common/Util.hpp>

#include <map>
#include <optional>
#include <regex>
#include <set>
#include <string>
#include <vector>

WopiHostList HostUtil::WopiHosts;
std::map<std::string, std::string> HostUtil::AliasHosts;
std::set<std::string> HostUtil::hostList;
std::string HostUtil::FirstHost;
bool HostUtil::FirstHostRejectionLogged;
bool HostUtil::WopiEnabled;

bool WopiHostList::Entry::matches(const std::string& host) const
{
    return network ? network->contains(host) : std::regex_match(host, regex);
}

void WopiHostList::add(const std::string& host, int port, bool allow)
{
    Entry entry;
    entry.network = Util::IpNetwork::parse(host);
    if (!entry.network)
    {
        try
        {
            entry.regex = std::regex(host, std::regex_constants::icase);
        }
        catch (const std::regex_error& exc)
        {
            LOG_WRN("Ignoring the WOPI host [" << host
                                               << "], which is not a valid regular expression: "
                                               << exc.what());
            return;
        }
    }
    entry.port = port;
    entry.allow = allow;
    _entries.push_back(std::move(entry));
}

void WopiHostList::allow(const std::string& host, int port)
{
    add(host, port, true);
}

void WopiHostList::deny(const std::string& host)
{
    add(host, AnyPort, false);
}

bool WopiHostList::match(const std::string& host, int port) const
{
    bool allowed = false;
    for (const Entry& entry : _entries)
    {
        if (!entry.matches(host))
            continue;
        // A blocked host is blocked on every port.
        if (!entry.allow)
            return false;
        if (port == AnyPort || entry.port == AnyPort || entry.port == port)
            allowed = true;
    }

    return allowed;
}

void HostUtil::parseWopiHost()
{
    // Parse the WOPI settings.
    WopiHosts.clear();
    WopiEnabled = ConfigUtil::getBool("storage.wopi[@allow]", false);
    if (WopiEnabled)
    {
        for (const std::string& path : ConfigUtil::getIndexedKeys("storage.wopi", "host"))
        {
            HostUtil::addWopiHost(ConfigUtil::getString(path, ""), 0,
                                  ConfigUtil::getBool(path + "[@allow]", false));
        }
    }
}

void HostUtil::addWopiHost(const std::string& host, int port, bool allow)
{
    if (host.empty())
        return;

    if (allow)
    {
        LOG_INF("Adding trusted WOPI host: [" << host << "] on "
                                              << (port ? "port " + std::to_string(port)
                                                       : std::string("any port")));
        WopiHosts.allow(host, port);
    }
    else
    {
        // A blocked host is blocked on every port, so that no entry allows it elsewhere.
        if (port)
            LOG_WRN("The blocked WOPI host [" << host << "] is blocked on every port, not only on "
                                              << port);
        LOG_INF("Adding blocked WOPI host: [" << host << "].");
        WopiHosts.deny(host);
    }
}

bool HostUtil::allowedWopiHost(const std::string& host)
{
    return WopiEnabled && WopiHosts.match(host);
}

bool HostUtil::allowedWopiHost(const std::string& host, int port)
{
    return WopiEnabled && WopiHosts.match(host, port);
}

std::string HostUtil::parseAlias(const std::string& aliasPattern)
{
    int port = 0;
    return parseAlias(aliasPattern, port);
}

std::string HostUtil::parseAlias(const std::string& aliasPattern, int& port)
{
    port = 0;
    if (!RegexUtil::isRegexValid(aliasPattern))
    {
        return {};
    }

    // check if it is plain uri, then convert to a strict regex for this uri if needed
    // Must be a full match.
    // Group 2 captures the hostname, group 5 the port with its colon.
    static const std::regex re(
        "^(https?://)?(([a-z0-9\\-]+)(\\.[a-z0-9\\-]+)*)(:[0-9]{1,5})?(/[a-z0-9\\-&?_]*)*$",
        std::regex_constants::icase);

    std::smatch matches;
    if (std::regex_match(aliasPattern, matches, re) && matches.size() > 2)
    {
        std::string hostname = matches[2].str();

        if (hostname.empty())
        {
            LOG_DBG("parseAlias: error could not find hostname in: " << aliasPattern);
            return {};
        }

        // make the hostname a regex matching itself
        Util::replaceAllSubStr(hostname, ".", "\\.");

        if (matches[5].matched)
            port = std::stoi(matches[5].str().substr(1));

        return hostname;
    }

    // A regular expression. Digits after its only colon are a port, so the documented form
    // aliasname.*\.com:port works; the colons of an IPv6 address are left alone.
    static const std::regex portRe("^([^:]+):([0-9]{1,5})$");
    if (std::regex_match(aliasPattern, matches, portRe) &&
        RegexUtil::isRegexValid(matches[1].str()))
    {
        port = std::stoi(matches[2].str());
        return matches[1].str();
    }

    return aliasPattern;
}

std::optional<Poco::URI> HostUtil::parseHostUri(const std::string& host)
{
    // The host and port are parsed out with Poco::URI, which needs a scheme to locate them. The
    // scheme is never used for matching, so a host may be written with or without one; when it is
    // missing, add a placeholder scheme purely so the value parses.
    const std::string uri = host.find("://") == std::string::npos ? "https://" + host : host;
    try
    {
        return Poco::URI(uri);
    }
    catch (const Poco::Exception& exc)
    {
        // Some regex hosts (an IPv6 literal, or a regex in the port) are not valid URIs. Skip such
        // a group rather than let the exception stop the whole configuration from loading.
        LOG_WRN("parseAliases: ignoring the alias_groups host ["
                << host << "] because it cannot be parsed as a URI: " << exc.displayText());
        return std::nullopt;
    }
}

void HostUtil::parseAliases(Poco::Util::LayeredConfiguration& conf)
{
    WopiEnabled = conf.getBool("storage.wopi[@allow]", false);

    //set alias_groups mode to compat
    if (!conf.has("storage.wopi.alias_groups"))
    {
        conf.setString("storage.wopi.alias_groups[@mode]", "compat");
        return;
    }

    if (conf.has("storage.wopi.alias_groups.group[0]"))
    {
        // group defined in alias_groups
        if (Util::iequal(ConfigUtil::getString("storage.wopi.alias_groups[@mode]", "first"),
                         "first"))
        {
            LOG_ERR("Admins did not set the alias_groups mode to 'groups'");
            AliasHosts.clear();
            return;
        }
    }
    else if (Util::iequal(ConfigUtil::getString("storage.wopi.alias_groups[@mode]", "first"),
                          "groups"))
    {
        LOG_WRN("alias_groups mode is 'groups' but no <group> is defined; all WOPI "
                "hosts will be denied. Define at least one group, or use mode='first'.");
    }

    AliasHosts.clear();
    WopiHosts.clear();
    hostList.clear();
#if ENABLE_FEATURE_LOCK
    CommandControl::LockManager::unlockLinkMap.clear();
#endif

    for (size_t i = 0;; i++)
    {
        const std::string path = "storage.wopi.alias_groups.group[" + std::to_string(i) + ']';
        if (!conf.has(path + ".host"))
        {
            break;
        }

        const std::string uri = conf.getString(path + ".host", "");
        if (uri.empty())
        {
            continue;
        }

        for (const std::string& extraHost : getExtraGroupHosts(conf, path))
        {
            LOG_WRN("alias_groups: ignoring <host> [" << extraHost << "] of group #" << i + 1
                                                       << ", which has [" << uri
                                                       << "] already. Put each host in a "
                                                          "<group> of its own, or make it an "
                                                          "<alias>");
        }

        const bool allow = conf.getBool(path + ".host[@allow]", false);

        const std::optional<Poco::URI> parsedHost = parseHostUri(uri);
        if (!parsedHost)
        {
            continue;
        }
        const Poco::URI& realUri = *parsedHost;

        try
        {
#if ENABLE_FEATURE_LOCK
            CommandControl::LockManager::mapUnlockLink(realUri.getHost(), path);
#endif
            HostUtil::hostList.insert(realUri.getHost());
            // A port counts only when it was written. parseHostUri adds a scheme, never a port.
            HostUtil::addWopiHost(parseAlias(realUri.getHost()), realUri.getSpecifiedPort(),
                                  allow);
        }
        catch (const Poco::Exception& exc)
        {
            LOG_WRN("parseAliases: " << exc.displayText());
        }

        static bool warnedInvalidRegex = false;
        static bool warnedAboutPipe = false;
        for (size_t j = 0;; j++)
        {
            const std::string aliasPath = path + ".alias[" + std::to_string(j) + ']';
            if (!conf.has(aliasPath))
            {
                break;
            }

            auto aliasString = conf.getString(aliasPath, "");
            if (aliasString.empty())
            {
                continue;
            }

            if (!warnedAboutPipe && aliasString.find('|') != std::string::npos)
            {
                LOG_WRN(
                    "Using | to define multiple alias is deprecated, it is recommended to use an "
                    "<alias> element for each alias. If the | is part of a regex disregard.");
            }

            for (std::string& aliasPattern : Util::splitStringToVector(aliasString, '|'))
            {
                int port = 0;
                aliasPattern = parseAlias(aliasPattern, port);
                if (aliasPattern.empty())
                {
                    if (!warnedInvalidRegex)
                        LOG_WRN("parseAliases: found invalid alias pattern: [" << aliasPattern << "] for uri" << uri);
                    continue;
                }
                LOG_DBG("parseAliases: Mapped URI alias ["
                        << aliasPattern << "] to canonical URI [" << realUri.getAuthority()
                        << ']');
                AliasHosts.emplace(aliasPattern, realUri.getAuthority());
#if ENABLE_FEATURE_LOCK
                CommandControl::LockManager::mapUnlockLink(aliasPattern, path);
#endif
                HostUtil::addWopiHost(aliasPattern, port, allow);
            }
        }
        warnedInvalidRegex = true;
        warnedAboutPipe = true;
    }
}

std::string HostUtil::getNewUri(const Poco::URI& uri)
{
    if (Util::iequal(ConfigUtil::getString("storage.wopi.alias_groups[@mode]", "first"), "compat"))
    {
        return uri.getPath();
    }

    Poco::URI newUri(uri);
    const std::string value = RegexUtil::getValue(AliasHosts, newUri.getAuthority());
    if (!value.empty())
    {
        newUri.setAuthority(value);
    }
    else
    {
        // It is allowed for the host to be a regex.
        // In that case, the first who connects is treated as the 'host', and stored to the AliasHosts here
        const std::string val = RegexUtil::getValue(hostList, newUri.getHost());
        // compare incoming request's host with existing hostList , if they are not equal it is regex and we store
        // the pair in AliasHosts
        if (!val.empty() && val.compare(newUri.getHost()) != 0)
        {
            LOG_DBG("Mapped URI alias [" << val << "] to canonical URI [" << newUri.getHost()
                                         << ']');
            AliasHosts.emplace(val, newUri.getHost());
        }
    }

    if (newUri.getAuthority().empty())
    {
        return newUri.getPath();
    }

    // Poco::URI::getHost() returns an IPv6 literal without its brackets, so bracket it back
    // here; otherwise the colons inside it would be indistinguishable from the port separator.
    const std::string& host = newUri.getHost();
    const std::string bracketedHost = host.find(':') != std::string::npos ? '[' + host + ']' : host;

    return newUri.getScheme() + "://" + bracketedHost + ':' + std::to_string(newUri.getPort()) +
           newUri.getPath();
}

Poco::URI HostUtil::getNewLockedUri(const Poco::URI& uri)
{
    Poco::URI newUri(uri);
    const std::string value = RegexUtil::getValue(AliasHosts, newUri.getAuthority());
    if (!value.empty())
    {
        newUri.setAuthority(value);
        LOG_WRN("The locked_host: " << uri.getAuthority() << " is alias of "
                                    << newUri.getAuthority() << ", Applying "
                                    << newUri.getAuthority() << " locked_host settings.");
    }

    return newUri;
}

std::vector<std::string> HostUtil::getExtraGroupHosts(const Poco::Util::AbstractConfiguration& conf,
                                                      const std::string& groupPath)
{
    std::vector<std::string> extraHosts;
    Poco::Util::AbstractConfiguration::Keys keys;
    conf.keys(groupPath, keys);
    for (const std::string& key : keys)
    {
        // The first is "host", the others "host[1]" and on.
        if (key.starts_with("host[") && key != "host[0]")
            extraHosts.push_back(conf.getString(groupPath + '.' + key, ""));
    }

    return extraHosts;
}

void HostUtil::setFirstHost(const Poco::URI& uri)
{
    if (!Util::iequal(ConfigUtil::getString("storage.wopi.alias_groups[@mode]", "first"), "first"))
        return;

    if (WopiHosts.empty())
    {
        if (FirstHost.empty())
        {
            FirstHost = uri.getHost();
            // The host comes from a request, so it is matched as that name only.
            addWopiHost(RegexUtil::escapeRegex(uri.getHost()), 0, true);
        }
    }
    else if(!FirstHost.empty() && FirstHost != uri.getHost())
    {
        if (FirstHostRejectionLogged)
        {
            LOG_DBG("Rejected WOPI host [" << uri.getHost() << "]: the only allowed host is ["
                                           << FirstHost << ']');
            return;
        }

        FirstHostRejectionLogged = true;
        LOG_ERR("Rejected WOPI host [" << uri.getHost() << "]: the only allowed host is ["
                                       << FirstHost
                                       << "]. To use multiple host/aliases check alias_groups tag "
                                          "in configuration. Later rejections are logged at debug "
                                          "level");
    }
}

bool HostUtil::isWopiHostsEmpty()
{
    return WopiHosts.empty();
}

bool HostUtil::isForbiddenKitHost(const std::string& host)
{
    // The same list that setLokitEnvironmentVariables hands to the kit: one
    // entry per line, each either a CIDR network or a regular expression.
    // Parsed on each call so that it follows the value the remote
    // configuration poll writes.
    const char* allowlist = std::getenv("KIT_HOST_ALLOWLIST");
    if (!allowlist || allowlist[0] == '\0')
        return false;

    std::string regex;
    for (const std::string& entry : Util::splitStringToVector(allowlist, '\n'))
    {
        if (std::optional<Util::IpNetwork> network = Util::IpNetwork::parse(entry))
        {
            if (network->contains(host))
                return false;
        }
        else
        {
            if (!regex.empty())
                regex += '|';
            regex += entry;
        }
    }

    if (regex.empty())
        return true;

    const std::regex allowedRegex(regex);
    return !std::regex_match(host, allowedRegex);
}

#endif // !MOBILEAPP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
