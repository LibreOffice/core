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

#include <QObject>
#include <QString>

/// Keeps the screen awake for as long as this object exists.
///
/// Uses the org.freedesktop.portal.Inhibit portal, which also works inside the
/// Flatpak sandbox, and falls back to org.freedesktop.ScreenSaver. D-Bus calls
/// are asynchronous and a failure is only logged. GUI thread only.
class ScreenInhibitor : public QObject
{
public:
    /// Starts inhibiting idle. The inhibit ends when the object is destroyed.
    ScreenInhibitor(const QString& reason, QObject* parent);
    ~ScreenInhibitor() override;

private:
    enum class Backend
    {
        None, ///< Nothing held or requested.
        Pending, ///< Request in flight.
        Portal, ///< Held through the Inhibit portal.
        ScreenSaver ///< Held through org.freedesktop.ScreenSaver.
    };

    void tryPortal(const QString& reason);
    void tryScreenSaver(const QString& reason);

    Backend _backend = Backend::Pending;

    // Portal request path; closing it ends the inhibit.
    QString _portalHandle;

    // Cookie from org.freedesktop.ScreenSaver.Inhibit. _backend says whether it is in use.
    unsigned int _screenSaverCookie = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
