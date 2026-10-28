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
#include <qt/qt.hpp>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QStringList>
#include <QUrl>
#include <QWebEngineUrlRequestJob>

#include <algorithm>

void CoolLocalSchemeHandler::requestStarted(QWebEngineUrlRequestJob* job)
{
    if (job->requestMethod() != "GET")
    {
        LOG_WRN("coollocal: refused: only GET requests are allowed");
        job->fail(QWebEngineUrlRequestJob::RequestDenied);
        return;
    }

    const QStringList segments = job->requestUrl().path().split('/', Qt::SkipEmptyParts);
    const auto isHidden = [](const QString& segment) { return segment.startsWith('.'); };
    const bool isPlainPath =
        !segments.isEmpty() && std::none_of(segments.begin(), segments.end(), isHidden);
    if (!isPlainPath)
    {
        LOG_WRN("coollocal: refused: the path is empty or has a part starting with a dot");
        job->fail(QWebEngineUrlRequestJob::RequestDenied);
        return;
    }

    const QDir browserDir(QString::fromStdString(getBrowserDistDir()));
    const QString browserPath = browserDir.canonicalPath() + '/';
    const QFileInfo file(browserDir.filePath(segments.join('/')));
    const bool isInBrowserDir = file.canonicalFilePath().startsWith(browserPath);
    if (!isInBrowserDir || !file.isFile())
    {
        LOG_DBG("coollocal: not found: the file is not in the browser folder");
        job->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }

    auto* device = new QFile(file.canonicalFilePath(), job);
    if (!device->open(QIODevice::ReadOnly))
    {
        LOG_WRN("coollocal: failed: the file in the browser folder cannot be read");
        job->fail(QWebEngineUrlRequestJob::RequestFailed);
        return;
    }

    const QMimeType type = QMimeDatabase().mimeTypeForFile(file, QMimeDatabase::MatchExtension);
    job->reply(type.name().toUtf8(), device);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
