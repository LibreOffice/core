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
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QSslCertificate>
#include <QSslKey>
#include <QStandardPaths>
#include <QStringList>
#include <QTemporaryFile>
#include <QUrl>
#include <QWebEngineProfile>

QWebEngineProfile* Application::globalProfile = nullptr;
RecentFiles Application::recentFiles;
QSslKey Application::embedKey;
QSslCertificate Application::embedCert;

namespace
{
/// Generate an ephemeral self-signed RSA cert + key valid for
/// loopback hosts, into the supplied @keyOut and @certOut, and
/// fill @spkiHashB64 with the base64-encoded SHA-256 of the cert's
/// SubjectPublicKeyInfo (the form Chromium's
/// --ignore-certificate-errors-spki-list flag wants).  Shells out
/// to the openssl CLI; OpenSSL is a transitive dependency of CODA's
/// existing TLS/HTTPS code so the binary is always present at
/// runtime on the platforms we ship to.
void generateEmbedCert(QSslKey& keyOut, QSslCertificate& certOut,
                       QByteArray& spkiHashB64)
{
    QTemporaryFile keyFile;
    QTemporaryFile certFile;
    if (!keyFile.open() || !certFile.open())
    {
        LOG_ERR("Application: cannot open tmp file for embed cert");
        return;
    }
    const QString keyPath = keyFile.fileName();
    const QString certPath = certFile.fileName();
    keyFile.close();
    certFile.close();

    QProcess openssl;
    openssl.start(QStringLiteral("openssl"), QStringList()
        << "req" << "-x509" << "-newkey" << "rsa:3072"
        << "-keyout" << keyPath << "-out" << certPath
        << "-days" << "1" << "-nodes"
        << "-subj" << "/CN=localhost"
        << "-addext"
        << "subjectAltName=DNS:localhost,IP:127.0.0.1,IP:::1"
        // Chromium requires EKU=serverAuth for HTTPS server certs;
        // a cert without it is rejected at handshake time, before
        // QWebEnginePage::certificateError gets a chance to override.
        << "-addext" << "extendedKeyUsage=serverAuth"
        << "-addext" << "basicConstraints=CA:FALSE"
        << "-addext" << "keyUsage=digitalSignature,keyEncipherment");
    if (!openssl.waitForFinished(10000) || openssl.exitCode() != 0)
    {
        LOG_ERR("Application: openssl failed generating embed cert: "
                << openssl.readAllStandardError().toStdString());
        QFile::remove(keyPath);
        QFile::remove(certPath);
        return;
    }

    QFile k(keyPath);
    QFile c(certPath);
    if (k.open(QIODevice::ReadOnly) && c.open(QIODevice::ReadOnly))
    {
        keyOut = QSslKey(k.readAll(), QSsl::Rsa);
        certOut = QSslCertificate(c.readAll(), QSsl::Pem);
    }

    // Compute base64(sha256(DER-encoded SubjectPublicKeyInfo)) of
    // the cert.  QSslCertificate has no direct accessor for the
    // SPKI, so chain a few openssl invocations.
    QProcess sh;
    sh.start(QStringLiteral("bash"), QStringList() << "-c" << QString(
        "openssl x509 -in '%1' -pubkey -noout | "
        "openssl pkey -pubin -outform DER | "
        "openssl dgst -sha256 -binary | base64").arg(certPath));
    if (sh.waitForFinished(10000) && sh.exitCode() == 0)
        spkiHashB64 = sh.readAllStandardOutput().trimmed();
    else
        LOG_ERR("Application: SPKI hash pipeline failed: "
                << sh.readAllStandardError().toStdString());

    QFile::remove(keyPath);
    QFile::remove(certPath);

    if (keyOut.isNull() || certOut.isNull() || spkiHashB64.isEmpty())
        LOG_ERR("Application: embed cert/key/spki parse failed");
}
}

void Application::initialize()
{
    if (!globalProfile)
    {
        // Generate the embed cert + SPKI hash and tell Chromium to
        // ignore cert errors for that specific cert via the SPKI
        // allowlist.  Must be set before the first profile/page is
        // created (which is what triggers Chromium init).  Scoped to
        // our cert only - the integrator's HTTPS origin is still
        // validated against the system trust store as normal.
        QByteArray spkiHash;
        generateEmbedCert(embedKey, embedCert, spkiHash);
        if (!spkiHash.isEmpty())
        {
            qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
                    "--ignore-certificate-errors-spki-list=" + spkiHash);
        }

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

const QSslKey& Application::getEmbedKey() { return embedKey; }

const QSslCertificate& Application::getEmbedCert() { return embedCert; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
