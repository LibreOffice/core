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

#include <typeinfo>
#include <Poco/Path.h>

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace Desktop
{
    struct FileResult
    {
        std::string fileName;
        std::string mimeType;
        std::string content;
    };

    Poco::Path getConfigPath();
    std::string getDataDir();

    void uploadSettings(const std::string& payload);

    FileResult fetchSettingsFile(const std::string& relPath);

    std::string fetchSettingsConfig();

    void syncSettings(const std::function<void(const std::vector<char>&)>& sendFileCallback);

    // Native-owned per-user UI preferences (preferences.json in the config dir).
    // getDarkMode() returns nullopt when the user has not chosen yet.
    std::optional<bool> getDarkMode();
    void setDarkMode(bool value);

    // Fetch the AI provider's model list (proxy for the desktop, which has no
    // server). payload is {"provider","apiKey","baseUrl"}; returns the provider's
    // JSON body or {"error":...}.
    std::string fetchAIModels(const std::string& payload);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
