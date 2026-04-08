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

#include <qt/RemoteOpen.hpp>

#include <qt/IntegratorFilePicker.hpp>
#include <qt/WebView.hpp>
#include <common/Log.hpp>

#include <QDir>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTemporaryFile>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QWebSocket>

namespace coda
{

void openRemoteFile(const QString& serverUrl, QWidget* parent,
                    QWebEngineProfile* profile)
{
    // Show the Nextcloud web UI.  When the user clicks a document,
    // richdocuments creates a COOL iframe whose URL contains the
    // WOPISrc and access_token.  NextcloudFilePicker intercepts that.
    IntegratorFilePicker picker(serverUrl, parent);
    if (picker.exec() != QDialog::Accepted)
        return;

    QString wopiSrc = picker.wopiSrc();
    QString accessToken = picker.accessToken();
    QString coolServer = picker.coolServer();
    QString coolPath = picker.coolPath();

    if (wopiSrc.isEmpty())
    {
        LOG_ERR("openRemoteFile: no WOPISrc from picker");
        return;
    }

    LOG_TRC("openRemoteFile: WOPISrc=" << wopiSrc.toStdString()
            << " coolServer=" << coolServer.toStdString());

    // Download the file via the COOL server's /co/collab endpoint.
    QNetworkAccessManager nam;
    QEventLoop loop;

    QString wsUrl = coolServer;
    wsUrl.replace("http://", "ws://");
    wsUrl.replace("https://", "wss://");
    wsUrl += "/co/collab?WOPISrc="
          + QUrl::toPercentEncoding(wopiSrc);

    auto* collabWs = new QWebSocket;
    auto* downloadUrl = new QString;
    auto* downloadFilename = new QString;
    auto* downloadDone = new bool(false);
    auto* collabLoop = new QEventLoop;
    auto* pendingMessages = new QStringList;

    QObject::connect(collabWs, &QWebSocket::connected,
        [collabWs, &accessToken]() {
            collabWs->sendTextMessage("access_token " + accessToken);
        });
    QObject::connect(collabWs, &QWebSocket::textMessageReceived,
        [downloadUrl, downloadFilename, downloadDone, collabWs, collabLoop, pendingMessages]
        (const QString& msg) {
            QJsonDocument jdoc = QJsonDocument::fromJson(msg.toUtf8());
            QJsonObject jobj = jdoc.object();
            QString type = jobj["type"].toString();

            if (type == "authenticated")
            {
                collabWs->sendTextMessage(
                    "{\"type\":\"fetch\","
                    "\"stream\":\"contents\","
                    "\"requestId\":\"coda-init\"}");
            }
            else if (type == "fetch_url"
                     && jobj["requestId"].toString() == "coda-init")
            {
                *downloadUrl = jobj["url"].toString();
                *downloadFilename = jobj["filename"].toString();
                *downloadDone = true;
                collabLoop->quit();
            }
            else if (type == "error" || type == "fetch_error")
            {
                LOG_ERR("openRemoteFile: collab error: "
                        << msg.toStdString());
                collabWs->close();
            }
            else
            {
                LOG_TRC("openRemoteFile: buffering collab msg: "
                        << msg.toStdString().substr(0, 100));
                pendingMessages->append(msg);
            }
        });
    QObject::connect(collabWs, &QWebSocket::disconnected,
        [downloadUrl, collabLoop]() {
            if (downloadUrl->isEmpty())
                collabLoop->quit();
        });

    QTimer collabTimeout;
    collabTimeout.setSingleShot(true);
    QObject::connect(&collabTimeout, &QTimer::timeout,
        [collabWs]() {
            LOG_ERR("openRemoteFile: collab WebSocket timeout");
            collabWs->close();
        });
    collabTimeout.start(30000);

    collabWs->open(QUrl(wsUrl));
    collabLoop->exec();

    if (downloadUrl->isEmpty())
    {
        LOG_ERR("openRemoteFile: failed to get download URL");
        delete downloadDone;
        delete downloadFilename;
        delete downloadUrl;
        delete collabLoop;
        delete pendingMessages;
        delete collabWs;
        return;
    }

    QString dlUrlStr = *downloadUrl;
    if (dlUrlStr.startsWith('/'))
        dlUrlStr = coolServer + dlUrlStr;

    QString filename = *downloadFilename;
    delete downloadDone;
    delete downloadFilename;
    delete downloadUrl;
    delete collabLoop;
    QStringList bufferedMessages = *pendingMessages;
    delete pendingMessages;

    // Download the file
    QUrl dlUrl(dlUrlStr);
    QNetworkRequest dlReq(dlUrl);
    QNetworkReply* dlReply = nam.get(dlReq);
    QObject::connect(dlReply, &QNetworkReply::finished,
                     &loop, &QEventLoop::quit);
    loop.exec();

    if (dlReply->error() != QNetworkReply::NoError)
    {
        LOG_ERR("openRemoteFile: download error: "
                << dlReply->errorString().toStdString());
        dlReply->deleteLater();
        delete collabWs;
        return;
    }

    // Preserve file extension for filter detection
    QString ext;
    int dot = filename.lastIndexOf('.');
    if (dot >= 0)
        ext = filename.mid(dot);
    auto* tmp = new QTemporaryFile(
        QDir::tempPath() + "/coda-XXXXXX" + ext);
    (void)tmp->open();
    tmp->write(dlReply->readAll());
    QString localPath = tmp->fileName();
    tmp->close();
    tmp->setAutoRemove(false);
    dlReply->deleteLater();

    // Disconnect download-phase handlers before transferring
    // ownership to RemoteDocInfo.
    collabWs->disconnect();

    auto remoteInfo = std::make_shared<RemoteDocInfo>();
    remoteInfo->wopiSrc = wopiSrc;
    remoteInfo->accessToken = accessToken;
    remoteInfo->coolServer = coolServer;
    remoteInfo->coolPath = coolPath;
    remoteInfo->collabWs.reset(collabWs);
    remoteInfo->pendingCollabMessages = std::move(bufferedMessages);

    WebView* webViewInstance = new WebView(profile);
    webViewInstance->loadRemote(localPath, std::move(remoteInfo));
}

} // namespace coda

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
