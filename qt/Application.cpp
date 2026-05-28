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
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QUrl>
#include <QWebEngineProfile>

QWebEngineProfile* Application::globalProfile = nullptr;
RecentFiles Application::recentFiles;

void Application::initialize()
{
    if (!globalProfile)
    {
        globalProfile = new QWebEngineProfile(QStringLiteral("PersistentProfile"));

        // Keep the WebEngine's persistent data (localStorage, cookies) under the
        // same directory as our settings instead of a separate data location.
        QString configData = QString::fromStdString(Desktop::getConfigPath().toString());
        QString cacheData = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

        globalProfile->setPersistentStoragePath(configData);
        globalProfile->setCachePath(cacheData);
        globalProfile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);

        globalProfile->installUrlSchemeHandler(
            "cool", new CoolUrlSchemeHandler(globalProfile));
    }

    // Initialize recent files
    Poco::Path configDir = Desktop::getConfigPath();
    recentFiles.load(configDir.append("RecentDocuments.conf").toString(), 15);

    // Provide AIChatSession with an HTTP transport (the COOL http::Session stack
    // isn't available here).
    registerAIHttpTransport();
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

std::string Desktop::fetchAIModels(const std::string& payload)
{
    const QJsonObject obj =
        QJsonDocument::fromJson(QByteArray::fromStdString(payload)).object();
    const QString provider = obj.value("provider").toString();
    const QString apiKey = obj.value("apiKey").toString();
    QString baseUrl = obj.value("baseUrl").toString();

    if (provider.isEmpty() || apiKey.isEmpty())
        return R"({"error":"Missing provider or apiKey"})";

    if (provider != "custom")
    {
        // Keep in sync with preCannedAIProviderBaseUrl() in wsd/FileServer.cpp.
        static const QMap<QString, QString> preCanned = {
            { "openai", "https://api.openai.com" },
            { "groq", "https://api.groq.com/openai" },
            { "together", "https://api.together.xyz" },
            { "mistral", "https://api.mistral.ai" },
        };
        baseUrl = preCanned.value(provider);
        if (baseUrl.isEmpty())
            return R"({"error":"Unknown provider"})";
    }
    else if (baseUrl.isEmpty())
    {
        return R"({"error":"Missing baseUrl for custom provider"})";
    }

    if (baseUrl.endsWith('/'))
        baseUrl.chop(1);
    baseUrl += "/v1/models";

    QNetworkAccessManager manager;
    QNetworkRequest request{ QUrl(baseUrl) };
    request.setRawHeader("Authorization", "Bearer " + apiKey.toUtf8());
    request.setRawHeader("Content-Type", "application/json");

    // Runs on the bridge call, so block (processing events) until the request
    // finishes and postMobileCall resolves with the result.
    QNetworkReply* reply = manager.get(request);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const QByteArray body = reply->readAll();
    const bool failed = reply->error() != QNetworkReply::NoError;
    reply->deleteLater();

    if (failed && body.isEmpty())
        return R"({"error":"Failed to reach the AI provider"})";

    // Return the provider's body verbatim ({"data":[...]} or its own error JSON).
    return body.toStdString();
}

QWebEngineProfile* Application::getProfile() { return globalProfile; }

RecentFiles& Application::getRecentFiles() { return recentFiles; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
