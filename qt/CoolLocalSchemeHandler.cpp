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

#include "CoolLocalSchemeHandler.hpp"
#include <common/Log.hpp>
#include <qt/RemoteOpen.hpp>
#include <qt/qt.hpp>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QStringList>
#include <QUrl>
#include <QUrlQuery>
#include <QWebEngineUrlRequestJob>

#include <algorithm>

namespace
{
/// The host of coollocal://message, where an editor frame sends its messages.
const QString MESSAGE_HOST = QStringLiteral("message");

/// Qt WebEngine cannot read a request body safely, so a message comes in the query of a GET.
void receiveMessage(QWebEngineUrlRequestJob* job)
{
    if (job->requestMethod() != "GET")
    {
        LOG_WRN("coollocal: refused: a message must be a GET request");
        job->fail(QWebEngineUrlRequestJob::RequestDenied);
        return;
    }

    const QUrlQuery query(job->requestUrl());
    const QString session = query.queryItemValue(QStringLiteral("session"));
    const QString kind = query.queryItemValue(QStringLiteral("kind"));
    const QString text = query.queryItemValue(QStringLiteral("message"), QUrl::FullyDecoded);
    const QByteArray message = text.toUtf8();
    coda::receiveEditorFrameMessage(job, session, kind, message);
}
}

QString coda::bundledFilePath(const QString& path)
{
    const QStringList segments = path.split('/', Qt::SkipEmptyParts);
    const auto isHidden = [](const QString& segment) { return segment.startsWith('.'); };
    const bool isPlainPath =
        !segments.isEmpty() && std::none_of(segments.begin(), segments.end(), isHidden);
    if (!isPlainPath)
    {
        LOG_WRN("coollocal: refused: the path is empty or has a part starting with a dot");
        return QString();
    }

    const QDir browserDir(QString::fromStdString(getBrowserDistDir()));
    const QString browserPath = browserDir.canonicalPath() + '/';
    const QFileInfo file(browserDir.filePath(segments.join('/')));
    const bool isInBrowserDir = file.canonicalFilePath().startsWith(browserPath);
    if (!isInBrowserDir || !file.isFile())
    {
        LOG_DBG("coollocal: not found: the file is not in the browser folder");
        return QString();
    }

    return file.canonicalFilePath();
}

void CoolLocalSchemeHandler::requestStarted(QWebEngineUrlRequestJob* job)
{
    if (job->requestUrl().host() == MESSAGE_HOST)
    {
        receiveMessage(job);
        return;
    }

    if (job->requestMethod() != "GET")
    {
        LOG_WRN("coollocal: refused: only GET requests are allowed");
        job->fail(QWebEngineUrlRequestJob::RequestDenied);
        return;
    }

    const QString filePath = coda::bundledFilePath(job->requestUrl().path());
    if (filePath.isEmpty())
    {
        job->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }

    auto* device = new QFile(filePath, job);
    if (!device->open(QIODevice::ReadOnly))
    {
        LOG_WRN("coollocal: failed: the file in the browser folder cannot be read");
        job->fail(QWebEngineUrlRequestJob::RequestFailed);
        return;
    }

    const QMimeType type =
        QMimeDatabase().mimeTypeForFile(filePath, QMimeDatabase::MatchExtension);
    job->reply(type.name().toUtf8(), device);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
