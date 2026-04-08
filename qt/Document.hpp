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

#include <Poco/URI.h>

#include <QString>
#include <QStringList>
#include <memory>

class QWebSocket;

namespace coda
{

/// Parameters for a remote document opened via a COOL server's
/// /co/collab endpoint.  Empty for local-only documents.
struct RemoteDocInfo
{
    QString wopiSrc;
    QString accessToken;
    QString coolServer;
    // The path component of the original COOL URL intercepted from
    // the integrator, e.g. "/browser/abc123/cool.html".  Needed
    // by switchToServerMode to navigate to the correct versioned
    // URL on the remote server.
    QString coolPath;
    // Heap-allocated; lives as long as the document is open.
    std::unique_ptr<QWebSocket> collabWs;
    // Collab messages received during download, before JS forwarding
    // is wired up.  Replayed by loadRemote.
    QStringList pendingCollabMessages;
};

struct DocumentData
{
    Poco::URI _fileURL;
    int _fakeClientFd = -1;
    unsigned _appDocId = 0;
    std::shared_ptr<RemoteDocInfo> _remoteInfo;
};

unsigned generateNewAppDocId();

} // namespace coda

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
