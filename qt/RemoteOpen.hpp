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

#pragma once

#include <QString>

#include <memory>

class QWidget;
class QWebEngineProfile;

namespace coda
{
struct RemoteDocInfo;

/// Result of downloading a document via /co/collab.
struct RemoteDownload
{
    /// Path to the downloaded temp file on disk.  Empty on failure.
    QString localPath;
    /// Populated RemoteDocInfo (wopiSrc, accessToken, coolServer/path,
    /// and the live collab WebSocket).  Null on failure.
    std::shared_ptr<RemoteDocInfo> remoteInfo;
};

/// Open a /co/collab WebSocket against coolServer, authenticate with
/// accessToken, request a fetch URL for the document identified by
/// wopiSrc, and download it to a temp file.  Runs a local event loop
/// (the caller's event loop is expected to be active).  Returns an
/// empty RemoteDownload on any step's failure.
RemoteDownload downloadRemoteDocument(const QString& wopiSrc,
                                      const QString& accessToken,
                                      const QString& coolServer,
                                      const QString& coolPath);

/// Show the remote file picker, extract WOPI parameters, download the
/// file via the COOL server's /co/collab endpoint, and open it in a
/// new WebView with the collab WebSocket attached for user
/// notifications.
void openRemoteFile(const QString& serverUrl, QWidget* parent,
                    QWebEngineProfile* profile);

} // namespace coda

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
