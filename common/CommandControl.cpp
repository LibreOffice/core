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
 * Feature lock and command control management.
 * Classes: LockManager - Manages locked commands and read-only hosts
 */

#include <config.h>

#include "CommandControl.hpp"

#include <common/ConfigUtil.hpp>
#include <common/RegexUtil.hpp>
#include <common/Util.hpp>

#include <string>
#include <unordered_set>

namespace CommandControl
{
bool LockManager::_isLockedUser = false;
bool LockManager::_isHostReadOnly = false;
std::unordered_set<std::string> LockManager::LockedCommandList;
std::string LockManager::LockedCommandListString;
RegexUtil::RegexListMatcher LockManager::readOnlyWopiHosts;
RegexUtil::RegexListMatcher LockManager::disabledCommandWopiHosts;
std::map<std::string, std::string> LockManager::unlockLinkMap;
bool LockManager::lockHostEnabled = false;
std::string LockManager::translationPath = std::string();
std::string LockManager::unlockLink = std::string();

void LockManager::generateLockedCommandList()
{
#ifdef ENABLE_FEATURE_LOCK

    LockedCommandListString = ConfigUtil::getString("feature_lock.locked_commands", "");
    Util::trim(LockedCommandListString);
    StringVector commandList = StringVector::tokenize(LockedCommandListString);

    std::string command;
    for (std::size_t i = 0; i < commandList.size(); i++)
    {
        command = commandList[i];
        if (!command.empty())
        {
            LockedCommandList.emplace(command);
        }
    }
#endif
}

const std::unordered_set<std::string>& LockManager::getLockedCommandList()
{
    if (LockedCommandList.empty())
        generateLockedCommandList();

    return LockedCommandList;
}

std::string LockManager::getLockedCommandListString()
{
    if (LockedCommandListString.empty())
        generateLockedCommandList();

    return LockedCommandListString;
}

void LockManager::parseLockedHost(Poco::Util::LayeredConfiguration& conf)
{
    readOnlyWopiHosts.clear();
    disabledCommandWopiHosts.clear();

    lockHostEnabled = ConfigUtil::getBool("feature_lock.locked_hosts[@allow]", false);

    if (lockHostEnabled)
    {
        for (size_t i = 0;; i++)
        {
            const std::string path = "feature_lock.locked_hosts.host[" + std::to_string(i) + ']';
            const std::string host = conf.getString(path, "");
            if (!host.empty())
            {
                if (conf.getBool(path + "[@read_only]", false))
                {
                    readOnlyWopiHosts.allow(host);
                }
                else
                {
                    readOnlyWopiHosts.deny(host);
                }

                if (conf.getBool(path + "[@disabled_commands]", false))
                {
                    disabledCommandWopiHosts.allow(host);
                }
                else
                {
                    disabledCommandWopiHosts.deny(host);
                }
            }
            else if (!conf.has(path))
            {
                break;
            }
        }
    }
}

bool LockManager::isHostReadOnly(const std::string& host)
{
    return LockManager::lockHostEnabled && LockManager::readOnlyWopiHosts.match(host);
}

bool LockManager::isHostCommandDisabled(const std::string& host)
{
    return LockManager::lockHostEnabled && LockManager::disabledCommandWopiHosts.match(host);
}

bool LockManager::hostExist(const std::string& host)
{
    return LockManager::lockHostEnabled && LockManager::readOnlyWopiHosts.matchExist(host);
}

void LockManager::setTranslationPath(const std::string& lockedDialogLang)
{
    for (size_t i = 0;; ++i)
    {
        const std::string path =
            "feature_lock.translations.language[" + std::to_string(i) + "][@name]";

        if (!ConfigUtil::has(path))
        {
            return;
        }
        if (ConfigUtil::getString(path, "") == lockedDialogLang)
        {
            LockManager::translationPath =
                "feature_lock.translations.language[" + std::to_string(i) + ']';
            return;
        }
    }
}
void LockManager::mapUnlockLink(const std::string& host, const std::string& path)
{
    if (!ConfigUtil::has(path + ".unlock_link"))
    {
        return;
    }
    const std::string link = ConfigUtil::getString(path + ".unlock_link", "");
    if (!link.empty())
    {
        unlockLinkMap.insert({host, link });
    }
}

bool RestrictionManager::_isRestrictedUser = false;
std::unordered_set<std::string> RestrictionManager::RestrictedCommandList;
std::string RestrictionManager::RestrictedCommandListString;

RestrictionManager::RestrictionManager() {}

void RestrictionManager::setRestrictedCommandList(const std::string& commandListString)
{
#if defined(ENABLE_FEATURE_RESTRICTION) || ENABLE_DEBUG
    RestrictedCommandListString = Util::trimmed(commandListString);
    RestrictedCommandList.clear();
#endif
}

const std::unordered_set<std::string>& RestrictionManager::getRestrictedCommandList()
{
    if (RestrictedCommandList.empty() && !RestrictedCommandListString.empty())
    {
        StringVector commandList = StringVector::tokenize(RestrictedCommandListString);
        for (std::size_t i = 0; i < commandList.size(); i++)
        {
            std::string command = commandList[i];
            if (!command.empty())
            RestrictedCommandList.emplace(command);
        }
    }

    return RestrictedCommandList;
}

std::string RestrictionManager::getRestrictedCommandListString()
{
#ifdef ENABLE_FEATURE_RESTRICTION
    if (RestrictedCommandListString.empty())
        setRestrictedCommandList(ConfigUtil::getString("restricted_commands", ""));
#endif

    return RestrictedCommandListString;
}
} // namespace CommandControl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
