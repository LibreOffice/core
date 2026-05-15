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

#include <common/MobileApp.hpp>
#include <qt/CoolUrlSchemeHandler.hpp>
#include <qt/qt.hpp>

#include <common/Log.hpp>
#include <common/RecentFiles.hpp>
#include <common/SettingsStorage.hpp>

#include <Poco/File.h>
#include <Poco/Path.h>

#include <QDir>
#include <QStandardPaths>
#include <QWebEngineProfile>

QWebEngineProfile* Application::globalProfile = nullptr;
RecentFiles Application::recentFiles;

void Application::initialize()
{
    if (!globalProfile)
    {
        globalProfile = new QWebEngineProfile(QStringLiteral("PersistentProfile"));

        QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QString cacheData = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

        globalProfile->setPersistentStoragePath(appData);
        globalProfile->setCachePath(cacheData);
        globalProfile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);

        globalProfile->installUrlSchemeHandler(
            "cool", new CoolUrlSchemeHandler(globalProfile));
    }

    // Initialize recent files
    Poco::Path configDir = Desktop::getConfigPath();
    recentFiles.load(configDir.append("RecentDocuments.conf").toString(), 15);
}

Poco::Path Desktop::getConfigPath()
{
    QString pathStr = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(pathStr);
    Poco::Path configPath(pathStr.toStdString());
    Poco::File configDir(configPath);
    if (!configDir.exists() || !configDir.isDirectory())
    {
        LOG_ERR("getConfigPath: following configuration directory does not exist, trouble ahead:"
                << pathStr.toStdString());
    }
    return configPath;
}

std::string Desktop::getDataDir()
{
    return ::getDataDir();
}

QWebEngineProfile* Application::getProfile() { return globalProfile; }

RecentFiles& Application::getRecentFiles() { return recentFiles; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
