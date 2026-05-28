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

// QNetworkAccessManager-based implementation of ai::HttpPostFn. The COOL net
// stack (http::Session) is server-only - the desktop app has no real outbound
// sockets through net/ - so AIChatSession reaches AI providers through this.

#include <config.h>

#include <qt/qt.hpp>

#include <common/AIHttpTransport.hpp>

#include <QByteArray>
#include <QMetaObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>
#include <QVariant>

#include <string>
#include <utility>

namespace
{
// One manager for all AI requests, created lazily on the Qt GUI thread.
QNetworkAccessManager* aiNetworkManager()
{
    static QNetworkAccessManager* manager = new QNetworkAccessManager(qApp);
    return manager;
}
} // namespace

void registerAIHttpTransport()
{
    ai::setHttpPostFn(
        [](const std::string& url, const std::string& authHeader, std::string body,
           int timeoutSeconds, ai::HttpDoneCallback onDone)
        {
            // Invoked on the document's polling thread; QNetworkAccessManager must be
            // driven by the GUI thread's event loop, so hop the request over there.
            const QString qUrl = QString::fromStdString(url);
            const QByteArray qAuth = QByteArray::fromStdString(authHeader);
            const QByteArray qBody = QByteArray::fromStdString(body);

            QMetaObject::invokeMethod(
                qApp,
                [qUrl, qAuth, qBody, timeoutSeconds, onDone = std::move(onDone)]() mutable
                {
                    QNetworkRequest request{ QUrl(qUrl) };
                    request.setRawHeader("Content-Type", "application/json");
                    if (!qAuth.isEmpty())
                        request.setRawHeader("Authorization", qAuth);
                    if (timeoutSeconds > 0)
                        request.setTransferTimeout(timeoutSeconds * 1000);

                    QNetworkReply* reply = aiNetworkManager()->post(request, qBody);

                    QObject::connect(
                        reply, &QNetworkReply::finished, qApp,
                        [reply, onDone = std::move(onDone)]()
                        {
                            const QVariant statusAttr =
                                reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

                            int statusCode;
                            if (statusAttr.isValid())
                                statusCode = statusAttr.toInt();
                            else if (reply->error() == QNetworkReply::TimeoutError ||
                                     reply->error() == QNetworkReply::OperationCanceledError)
                                statusCode = ai::HttpNoResponse;
                            else
                                statusCode = ai::HttpConnectFailed;

                            std::string responseBody = reply->readAll().toStdString();
                            reply->deleteLater();

                            onDone(statusCode, std::move(responseBody));
                        });
                },
                Qt::QueuedConnection);
        });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
