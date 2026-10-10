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
#include <qt/QtClipboard.hpp>
#include <qt/qt.hpp>

#include <common/Log.hpp>
#include <common/Prefs.hpp>
#include <common/RecentFiles.hpp>
#include <common/SettingsStorage.hpp>

#include <Poco/File.h>
#include <Poco/Path.h>

#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>
#include <QWebEngineCookieStore>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

namespace
{
QWebEngineProfile* createSavedProfile(const QString& storageName, const QString& storagePath,
                                      const QString& cachePath)
{
    LOG_DBG("Application: profile '" << storageName.toStdString() << "' keeps its files in '"
                                     << storagePath.toStdString() << "' and its cache in '"
                                     << cachePath.toStdString() << "'");

    auto* profile = new QWebEngineProfile(storageName);
    profile->setPersistentStoragePath(storagePath);
    profile->setCachePath(cachePath);
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    return profile;
}

// The id names a folder on disk, so it is taken only as one plain path segment.
bool isPlainAccountId(const QString& accountId)
{
    static const QRegularExpression plainId(
        QRegularExpression::anchoredPattern(QStringLiteral("[A-Za-z0-9_-]{1,64}")));
    return plainId.match(accountId).hasMatch();
}

QString accountStorageRoot()
{
    return QString::fromStdString(Desktop::getConfigPath().toString()) +
           QStringLiteral("/accounts");
}

QString accountCacheRoot()
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
           QStringLiteral("/accounts");
}

QString accountStoragePath(const QString& accountId)
{
    return accountStorageRoot() + '/' + accountId;
}

QString accountCachePath(const QString& accountId)
{
    return accountCacheRoot() + '/' + accountId;
}

// A folder goes only when its real path is the accounts folder's own plus the id, so a
// symbolic link in its place leaves everything where it is.
void removeAccountFolders(const QString& accountId)
{
    for (const QString& accounts : { accountStorageRoot(), accountCacheRoot() })
    {
        const QString folder = accounts + '/' + accountId;
        if (!QFileInfo::exists(folder))
        {
            LOG_DBG("Application: '" << folder.toStdString()
                                      << "' does not exist, so there is nothing to remove");
            continue;
        }

        if (QFileInfo(folder).canonicalFilePath() !=
            QDir(accounts).canonicalPath() + '/' + accountId)
        {
            LOG_ERR("Application: refusing to remove '" << folder.toStdString()
                                                        << "': not an account folder");
            continue;
        }

        LOG_DBG("Application: removing '" << folder.toStdString() << "'");
        if (!QDir(folder).removeRecursively())
            LOG_DBG("Application: could not remove all of '" << folder.toStdString() << "'");
    }
}

// A profile has to outlive every page built on it, so each one stays until CODA exits.
QMap<QString, QWebEngineProfile*>& accountProfiles()
{
    static QMap<QString, QWebEngineProfile*> profiles;
    return profiles;
}
}

QWebEngineProfile* Application::globalProfile = nullptr;
RecentFiles Application::recentFiles;
std::unique_ptr<Prefs> Application::prefs;

void Application::initialize()
{
    if (!globalProfile)
    {
        // Keep the WebEngine's persistent data (localStorage, cookies) under the
        // same directory as our settings instead of a separate data location.
        QString configData = QString::fromStdString(Desktop::getConfigPath().toString());
        QString cacheData = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

        globalProfile = createSavedProfile(QStringLiteral("PersistentProfile"), configData,
                                           cacheData);

        globalProfile->installUrlSchemeHandler(
            "cool", new CoolUrlSchemeHandler(globalProfile));

        // The remote-doc flow loads cool.html via file://
        // so its page-side JS can fetch() the cool server's
        // /co/collab/fetch and POST to /co/collab/put.  Chromium
        // blocks file://-origin pages from accessing remote URLs by
        // default; flipping LocalContentCanAccessRemoteUrls lets the
        // cross-origin fetches through (CORS on the server side is
        // what controls actual access).
        globalProfile->settings()->setAttribute(
            QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    }

    // Initialize recent files
    const Poco::Path configDir = Desktop::getConfigPath();
    recentFiles.load(Poco::Path(configDir).append("RecentDocuments.conf").toString(), 15);

    // Initialize persistent prefs
    prefs = std::make_unique<Prefs>(
        Poco::Path(configDir).append("coda-prefs.json").toString());

    // Provide AIChatSession with an HTTP transport (the COOL http::Session stack
    // isn't available here).
    registerAIHttpTransport();

    initializeQtClipboard();
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

QWebEngineProfile* Application::getAccountProfile(const QString& accountId)
{
    if (!isPlainAccountId(accountId))
    {
        LOG_DBG("Application: account id '" << accountId.toStdString()
                                             << "' is not plain, so it gets the shared profile");
        return globalProfile;
    }

    auto it = accountProfiles().find(accountId);
    if (it != accountProfiles().end())
        return it.value();

    QWebEngineProfile* profile =
        createSavedProfile(QStringLiteral("Account-") + accountId,
                           accountStoragePath(accountId), accountCachePath(accountId));
    accountProfiles().insert(accountId, profile);
    return profile;
}

int Application::countAccountProfileViews(const QString& accountId)
{
    auto it = accountProfiles().find(accountId);
    if (it == accountProfiles().end())
    {
        LOG_DBG("Application: account '" << accountId.toStdString()
                                        << "' has no profile this session, so no views use it");
        return 0;
    }

    int count = 0;
    for (QWidget* widget : QApplication::allWidgets())
    {
        auto* view = qobject_cast<QWebEngineView*>(widget);
        if (view && view->page() && view->page()->profile() == it.value())
            ++count;
    }
    return count;
}

void Application::deleteAccountProfile(const QString& accountId)
{
    if (!isPlainAccountId(accountId))
    {
        LOG_DBG("Application: account id '" << accountId.toStdString()
                                             << "' is not plain, so nothing is deleted");
        return;
    }

    // A profile in use this session holds its files open, so it is emptied through Qt instead.
    auto it = accountProfiles().find(accountId);
    if (it != accountProfiles().end())
    {
        LOG_DBG("Application: account '" << accountId.toStdString()
                                        << "' is in use, so only its cookies and cache go");
        it.value()->cookieStore()->deleteAllCookies();
        it.value()->clearHttpCache();
        return;
    }

    removeAccountFolders(accountId);
}

RecentFiles& Application::getRecentFiles() { return recentFiles; }

Prefs& Application::getPrefs() { return *prefs; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
