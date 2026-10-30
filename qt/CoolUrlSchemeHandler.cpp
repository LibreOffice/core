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

    // Each value is decoded in full, as the server does for a media request, and getDocKey decodes
    // the WOPISrc once more. A file name with a space or parentheses is encoded once more than the
    // rest of the WOPISrc, and only then matches the key of its DocumentBroker.
    const QUrlQuery query(url.query(QUrl::FullyEncoded));
    const std::string wopiSrc = query.queryItemValue("WOPISrc", QUrl::FullyDecoded).toStdString();
    const std::string tag = query.queryItemValue("Tag", QUrl::FullyDecoded).toStdString();

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

    // Chromium does not run its cross-origin check on a reply from a custom scheme handler, so
    // <video crossOrigin="anonymous"> loads the reply without an Access-Control-Allow-Origin
    // header.
    job->reply(isVtt ? "text/vtt" : "application/octet-stream", file);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
