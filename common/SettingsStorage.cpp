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

#include <config.h>

#include "SettingsStorage.hpp"

#include <common/ConfigUtil.hpp>
#include <common/Log.hpp>
#include <common/Util.hpp>

#include <Poco/DirectoryIterator.h>
#include <Poco/File.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include <exception>
#include <fstream>
#include <sstream>

namespace Desktop
{

void uploadSettings(const std::string& payload)
{
    try
    {
        Poco::JSON::Parser parser;
        const auto obj = parser.parse(payload).extract<Poco::JSON::Object::Ptr>();

        const std::string fileName = obj->getValue<std::string>("fileName");
        const std::string filePath = obj->getValue<std::string>("filePath");
        const std::string content = obj->getValue<std::string>("content");

        Poco::Path target = getConfigPath();
        target.append(filePath).append(fileName);

        Poco::File(target.parent()).createDirectories();

        std::ofstream out(target.toString(), std::ios::trunc);
        if (!out.is_open())
        {
            LOG_ERR("uploadSettings failed: Could not open " << target.toString()
                                                             << " for writing");
            return;
        }
        out << content;
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("uploadSettings failed: " << ex.what());
    }
}

FileResult fetchSettingsFile(const std::string& relPath)
{
    Poco::Path target = getConfigPath();
    target.append(relPath);

    if (!Poco::File(target).exists())
        return {};

    std::ifstream in(target.toString(), std::ios::binary);
    if (!in.is_open())
    {
        LOG_ERR("fetchSettingsFile failed: Could not open " << target.toString());
        return {};
    }
    std::ostringstream ss;
    ss << in.rdbuf();

    return { target.getFileName(), "text/plain", ss.str() };
}

static void traverseSettings(const Poco::Path& configPath, const Poco::Path& current,
                             Poco::JSON::Object::Ptr json)
{
    Poco::File dir(current);

    if (!dir.exists() || !dir.isDirectory())
        return;

    Poco::DirectoryIterator end;
    for (Poco::DirectoryIterator it(dir); it != end; ++it)
    {
        if (it->isDirectory())
        {
            traverseSettings(configPath, Poco::Path(it->path()), json);
        }

        else if (it->isFile())
        {
            Poco::Path filePath(it->path());
            std::string filePathStr = filePath.toString();
            std::string configPathStr = configPath.toString();

            if (configPathStr.back() != Poco::Path::separator())
                configPathStr += Poco::Path::separator();

            if (filePathStr.find(configPathStr) == 0)
            {
                std::string relPath = filePathStr.substr(configPathStr.length());
                Poco::Path p(relPath);

                if (p.depth() > 0)
                {
                    std::string key = p.directory(p.depth() - 1);
                    Poco::JSON::Object::Ptr item = new Poco::JSON::Object();
                    item->set("stamp", "");
                    item->set("uri", relPath);
                    Poco::JSON::Array::Ptr arr;
                    if (json->has(key))
                        arr = json->getArray(key);
                    else
                    {
                        arr = new Poco::JSON::Array();
                        json->set(key, arr);
                    }
                    arr->add(item);
                }
            }
        }
    }
}

std::string fetchSettingsConfig()
{
    Poco::JSON::Object::Ptr obj = new Poco::JSON::Object();
    obj->set("kind", "user");
    try
    {
        Poco::Path configPath = getConfigPath();
        Poco::Path settingsPath = configPath;
        settingsPath.append("settings");
        traverseSettings(configPath, settingsPath, obj);
        std::ostringstream oss;
        obj->stringify(oss);
        return oss.str();
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("fetchSettingsConfig failed: " << ex.what());
        return "{\"kind\":\"user\"}";
    }
}

void syncSettings(const std::function<void(const std::vector<char>&)>& sendFileCallback)
{
    try
    {
        Poco::Path base = getConfigPath();
        std::string settingsConfig = fetchSettingsConfig();

        Poco::JSON::Parser parser;
        const Poco::JSON::Object::Ptr json =
            parser.parse(settingsConfig).extract<Poco::JSON::Object::Ptr>();

        auto sendFile = [&](const std::string& key) {
            if (json->has(key))
            {
                Poco::JSON::Array::Ptr arr = json->getArray(key);
                if (arr->size() > 0)
                {
                    Poco::JSON::Object::Ptr item = arr->getObject(0);
                    std::string uri = item->getValue<std::string>("uri");
                    Poco::Path path = base;
                    path.append(uri);

                    if (Poco::File(path).exists())
                    {
                        std::ifstream in(path.toString(), std::ios::binary);
                        if (!in.is_open())
                        {
                            LOG_ERR("syncSettings failed: Could not open file for key '"
                                    << key << "' at " << path.toString());
                            return;
                        }
                        std::ostringstream ss;
                        ss << in.rdbuf();

                        const std::string payload = key + ": " + ss.str();

                        sendFileCallback(std::vector<char>(payload.begin(), payload.end()));
                    }
                    return;
                }
            }
            LOG_TRC("syncSettings failed: no " << key << " found");
        };

        sendFile("viewsetting");
        // browsersetting.json is unused on the desktop apps: the Options
        // dialog's Interface Settings section is hidden, View-menu toggles
        // persist via localStorage, and AI/Zotero/signature credentials live
        // in viewsetting.json. Skip the sync to avoid stale values shadowing
        // anything.
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("syncSettings failed: " << ex.what());
    }
}

static Poco::Path preferencesPath()
{
    Poco::Path path = getConfigPath();
    path.append("preferences.json");
    return path;
}

std::optional<bool> getDarkMode()
{
    try
    {
        const Poco::Path path = preferencesPath();
        if (!Poco::File(path).exists())
            return std::nullopt;

        std::ifstream in(path.toString(), std::ios::binary);
        if (!in.is_open())
            return std::nullopt;
        std::ostringstream ss;
        ss << in.rdbuf();

        Poco::JSON::Parser parser;
        const auto obj = parser.parse(ss.str()).extract<Poco::JSON::Object::Ptr>();
        if (obj && obj->has("darkMode"))
            return obj->getValue<bool>("darkMode");
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("getDarkMode failed: " << ex.what());
    }
    return std::nullopt;
}

void setDarkMode(bool value)
{
    try
    {
        const Poco::Path path = preferencesPath();

        // Merge into any existing preferences so other keys are preserved.
        Poco::JSON::Object::Ptr obj;
        if (Poco::File(path).exists())
        {
            std::ifstream in(path.toString(), std::ios::binary);
            std::ostringstream ss;
            ss << in.rdbuf();
            try
            {
                Poco::JSON::Parser parser;
                obj = parser.parse(ss.str()).extract<Poco::JSON::Object::Ptr>();
            }
            catch (const std::exception&)
            {
                obj = nullptr;
            }
        }
        if (!obj)
            obj = new Poco::JSON::Object();

        obj->set("darkMode", value);

        Poco::File(path.parent()).createDirectories();
        std::ofstream out(path.toString(), std::ios::trunc);
        if (!out.is_open())
        {
            LOG_ERR("setDarkMode failed: could not open " << path.toString());
            return;
        }
        obj->stringify(out);
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("setDarkMode failed: " << ex.what());
    }
}

} // namespace Desktop
