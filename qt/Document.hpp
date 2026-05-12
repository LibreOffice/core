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
#include <memory>

namespace coda
{

/// WOPI connection params for a remote document opened via a COOL
/// server's /co/collab endpoint.  Empty for local-only documents.
/// The collab WebSocket itself is owned by the page-JS, not the
/// native side; this struct only carries what the page-JS needs to
/// reconstruct the URL at bootstrap (and what
/// switchToServerMode-style flows need post-load).
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
