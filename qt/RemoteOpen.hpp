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

/// Show the remote file picker; once the user picks a document, open
/// a WebView pointing at cool.html with the picker-extracted WOPI
/// params attached to the WebView's RemoteDocInfo.  The page-JS does
/// the /co/collab dance (fetch_url, GET bytes, write temp file via
/// Bridge::writeRemoteDocFile) before the standard load flow runs.
void openRemoteFile(const QString& serverUrl, QWidget* parent,
                    QWebEngineProfile* profile);

} // namespace coda

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
