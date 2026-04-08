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

class QWidget;
class QWebEngineProfile;

namespace coda
{

/// Show the remote file picker, extract WOPI parameters, download the
/// file via the COOL server's /co/collab endpoint, and open it in a
/// new WebView with the collab WebSocket attached for user
/// notifications.
void openRemoteFile(const QString& serverUrl, QWidget* parent,
                    QWebEngineProfile* profile);

} // namespace coda

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
