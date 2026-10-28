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

#include <QByteArray>
#include <QString>

#include <optional>

class QUrl;
class QWidget;
class QWebEngineProfile;

namespace coda
{

/// Takes a message that an editor frame sent to coollocal://message. The frame names its
/// session, and the message must come from the origin of the frame given that session. Returns
/// the reply, or nothing when the message is refused.
std::optional<QByteArray> receiveEditorFrameMessage(const QString& session, const QUrl& origin,
                                                    const QString& kind, const QByteArray& message);

/// Opens the integrator at serverUrl in a window on pickerProfile. A document opened there loads
/// the COOL server's editor in the integrator's frame, as in a browser.
void openRemoteFile(const QString& serverUrl, QWidget* parent,
                    QWebEngineProfile* profile, QWebEngineProfile* pickerProfile);

} // namespace coda

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
