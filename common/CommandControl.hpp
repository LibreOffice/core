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

#pragma once

#include <common/ConfigUtil.hpp>
#include <common/Log.hpp>
#include <common/RegexUtil.hpp>
#include <common/Util.hpp>

#include <Poco/Exception.h>
#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <string>
#include <unordered_set>

namespace CommandControl
{
class LockManager
{
    static std::unordered_set<std::string> LockedCommandList;
    static bool _isLockedUser;
    static bool _isHostReadOnly;
    static std::string LockedCommandListString;
    static std::string translationPath;

    static void generateLockedCommandList();

public:
    LockManager() = delete;
    static const std::unordered_set<std::string>& getLockedCommandList();
    static std::string getLockedCommandListString();

    // Allow/deny Locked hosts
    static RegexUtil::RegexListMatcher readOnlyWopiHosts;
    static RegexUtil::RegexListMatcher disabledCommandWopiHosts;
    static std::map<std::string, std::string> unlockLinkMap;
    static std::string unlockLink;
    static bool lockHostEnabled;

    static void parseLockedHost(Poco::Util::LayeredConfiguration& conf);

    static bool isLockedUser() { return _isLockedUser; }
    static bool isLockReadOnly()
    {
        return ConfigUtil::getBool("feature_lock.is_lock_readonly", false) || isHostReadOnly();
    }
    static bool isHostReadOnly() { return _isHostReadOnly; };
    static bool isLockedReadOnlyUser() { return isLockedUser() && isLockReadOnly(); }

    static bool isHostReadOnly(const std::string& host);
    static bool isHostCommandDisabled(const std::string& host);
    static bool hostExist(const std::string& host);

    static void setLockedUser(bool isLocked) { _isLockedUser = isLocked; }
    static void setHostReadOnly(bool isReadOnly) { _isHostReadOnly = isReadOnly; }
    static void setTranslationPath(const std::string& lockedDialogLang);
    static std::string getUnlockTitle()
    {
        if (ConfigUtil::has(translationPath + ".unlock_title"))
            return ConfigUtil::getString(translationPath + ".unlock_title", "");
        return ConfigUtil::getString("feature_lock.unlock_title", "");
    }
    static std::string getUnlockLink()
    {
        if (!unlockLink.empty())
            return unlockLink;
        return ConfigUtil::getString("feature_lock.unlock_link", "");
    }
    static std::string getUnlockDescription()
    {
        if (ConfigUtil::has(translationPath + ".unlock_description"))
            return ConfigUtil::getString(translationPath + ".unlock_description", "");
        return ConfigUtil::getString("feature_lock.unlock_description", "");
    }
    static std::string getWriterHighlights()
    {
        if (ConfigUtil::has(translationPath + ".writer_unlock_highlights"))
            return ConfigUtil::getString(translationPath + ".writer_unlock_highlights", "");
        return ConfigUtil::getString("feature_lock.writer_unlock_highlights", "");
    }
    static std::string getCalcHighlights()
    {
        if (ConfigUtil::has(translationPath + ".calc_unlock_highlights"))
            return ConfigUtil::getString(translationPath + ".calc_unlock_highlights", "");
        return ConfigUtil::getString("feature_lock.calc_unlock_highlights", "");
    }
    static std::string getImpressHighlights()
    {
        if (ConfigUtil::has(translationPath + ".impress_unlock_highlights"))
            return ConfigUtil::getString(translationPath + ".impress_unlock_highlights", "");
        return ConfigUtil::getString("feature_lock.impress_unlock_highlights", "");
    }
    static std::string getDrawHighlights()
    {
        if (ConfigUtil::has(translationPath + ".draw_unlock_highlights"))
            return ConfigUtil::getString(translationPath + ".draw_unlock_highlights", "");
        return ConfigUtil::getString("feature_lock.draw_unlock_highlights", "");
    }
    static Poco::URI getUnlockImageUri()
    {
        const std::string unlockImageUrl = ConfigUtil::getString("feature_lock.unlock_image", "");
        if (!unlockImageUrl.empty())
        {
            try
            {
                const Poco::URI unlockImageUri(unlockImageUrl);
                return unlockImageUri;
            }
            catch (Poco::SyntaxException& exc)
            {
                LOG_ERR("Parsing of unlock_image url failed with " << exc.what());
            }
        }
        return Poco::URI();
    }
    static void resetTransalatioPath() { translationPath = std::string(); }
    static void mapUnlockLink(const std::string& host, const std::string& path);
    static void setUnlockLink(const std::string& host)
    {
        unlockLink = RegexUtil::getValue(unlockLinkMap, host);
    }
};

class RestrictionManager
{
    static std::unordered_set<std::string> RestrictedCommandList;
    static bool _isRestrictedUser;
    static std::string RestrictedCommandListString;

public:
    RestrictionManager();
    static void setRestrictedCommandList(const std::string& commandListString);
    static const std::unordered_set<std::string>& getRestrictedCommandList();
    static std::string getRestrictedCommandListString();

    static bool isRestrictedUser() { return _isRestrictedUser; }

    static void setRestrictedUser(bool isRestrictedUser) { _isRestrictedUser = isRestrictedUser; }
};
} // namespace CommandControl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
