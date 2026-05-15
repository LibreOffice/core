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

#include "CoolUrlSchemeHandler.hpp"

#include <common/Log.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/RequestDetails.hpp>

#include <QByteArray>
#include <QFile>
#include <QMultiMap>
#include <QUrl>
#include <QUrlQuery>
#include <QWebEngineUrlRequestJob>

#include <map>
#include <memory>
#include <mutex>
#include <string>

extern std::map<std::string, std::shared_ptr<DocumentBroker>> DocBrokers;
extern std::mutex DocBrokersMutex;

void CoolUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob* job)
{
    const QUrl url = job->requestUrl();
    const bool isVtt = (url.path() == QStringLiteral("/cool/mediavtt"));

    // The server percent-encodes the '&' query separators (see
    // ClientSession::createPublicURI), so decode once before parsing.
    QUrlQuery query;
    query.setQuery(QUrl::fromPercentEncoding(url.query(QUrl::FullyEncoded).toUtf8()));
    const std::string wopiSrc = query.queryItemValue("WOPISrc").toStdString();
    const std::string tag = query.queryItemValue("Tag").toStdString();

    std::shared_ptr<DocumentBroker> docBroker;
    {
        std::lock_guard<std::mutex> lock(DocBrokersMutex);
        const auto it = DocBrokers.find(RequestDetails::getDocKey(wopiSrc));
        if (it != DocBrokers.end())
            docBroker = it->second;
    }

    if (!docBroker)
    {
        LOG_ERR_S("No DocBroker for WOPISrc [" << wopiSrc << "] tag [" << tag << ']');
        job->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }

    const std::string mediaPath = docBroker->getEmbeddedMediaPath(tag);
    if (mediaPath.empty())
    {
        LOG_ERR_S("No media path for tag [" << tag << "] WOPISrc [" << wopiSrc << ']');
        job->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }

    auto* file = new QFile(QString::fromStdString(mediaPath), job);
    if (!file->open(QIODevice::ReadOnly))
    {
        LOG_ERR_S("Failed to open media file [" << mediaPath << "] for tag [" << tag
                  << "]: " << file->errorString().toStdString());
        job->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }

    // file:// pages have origin "null"; required for <video crossOrigin="anonymous">.
    QMultiMap<QByteArray, QByteArray> headers;
    headers.insert("Access-Control-Allow-Origin", "null");
    job->setAdditionalResponseHeaders(headers);

    job->reply(isVtt ? "text/vtt" : "application/octet-stream", file);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
