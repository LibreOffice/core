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

#include "ScreenInhibitor.hpp"

#include <common/Log.hpp>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QPointer>
#include <QVariantList>
#include <QVariantMap>

namespace
{
// Flags of org.freedesktop.portal.Inhibit.Inhibit: 8 = inhibit the session
// going idle (screen blanking and the idle lock).
constexpr unsigned int PORTAL_INHIBIT_IDLE = 8;

// Calls may wait on D-Bus activation. They are asynchronous, so this only
// bounds how long we wait for a reply, never how long the UI is blocked.
constexpr int DBUS_TIMEOUT_MS = 3000;

// Fire-and-forget: ends a portal inhibit. Safe to call from a destructor.
void closePortalRequest(const QString& handle)
{
    // During shutdown the bus connection may already be torn down; the portal
    // drops the inhibit itself when our connection closes.
    if (!QCoreApplication::instance())
        return;

    QDBusMessage message = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.portal.Desktop"), handle,
        QStringLiteral("org.freedesktop.portal.Request"), QStringLiteral("Close"));
    QDBusConnection::sessionBus().asyncCall(message, DBUS_TIMEOUT_MS);
    LOG_INF("Released the idle inhibit through the Inhibit portal");
}

// Fire-and-forget: ends an org.freedesktop.ScreenSaver inhibit.
void unInhibitScreenSaver(unsigned int cookie)
{
    // See closePortalRequest(): the service drops the inhibit when we disconnect.
    if (!QCoreApplication::instance())
        return;

    QDBusMessage message = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.ScreenSaver"),
        QStringLiteral("/org/freedesktop/ScreenSaver"),
        QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("UnInhibit"));
    message.setArguments(QVariantList{ cookie });
    QDBusConnection::sessionBus().asyncCall(message, DBUS_TIMEOUT_MS);
    LOG_INF("Released the idle inhibit through org.freedesktop.ScreenSaver");
}
} // namespace

ScreenInhibitor::ScreenInhibitor(const QString& reason, QObject* parent)
    : QObject(parent)
{
    tryPortal(reason);
}

ScreenInhibitor::~ScreenInhibitor()
{
    switch (_backend)
    {
        case Backend::Portal:
            closePortalRequest(_portalHandle);
            break;
        case Backend::ScreenSaver:
            unInhibitScreenSaver(_screenSaverCookie);
            break;
        case Backend::Pending:
            LOG_INF("Cancelled a pending idle inhibit request");
            break;
        case Backend::None:
            break;
    }
}

void ScreenInhibitor::tryPortal(const QString& reason)
{
    QDBusMessage message = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.portal.Desktop"),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.Inhibit"), QStringLiteral("Inhibit"));

    QVariantMap options;
    options.insert(QStringLiteral("reason"), reason);

    // The window identifier is optional; an empty one means no parent window.
    message.setArguments(QVariantList{ QString(), PORTAL_INHIBIT_IDLE, options });

    // Not parented to this object: a reply that arrives after it is destroyed
    // must still be handled, so the inhibit can be closed.
    auto* watcher = new QDBusPendingCallWatcher(
        QDBusConnection::sessionBus().asyncCall(message, DBUS_TIMEOUT_MS));

    QPointer<ScreenInhibitor> self(this);
    QObject::connect(
        watcher, &QDBusPendingCallWatcher::finished, watcher,
        [self, reason](QDBusPendingCallWatcher* call)
        {
            call->deleteLater();
            QDBusPendingReply<QDBusObjectPath> reply = *call;

            if (reply.isValid())
            {
                const QString handle = reply.value().path();
                if (self)
                {
                    self->_portalHandle = handle;
                    self->_backend = Backend::Portal;
                    LOG_INF("Inhibiting idle through the Inhibit portal: "
                            << handle.toStdString());
                }
                else
                {
                    // Destroyed while the request was in flight.
                    closePortalRequest(handle);
                }
                return;
            }

            LOG_INF("Inhibit portal not usable: " << reply.error().message().toStdString());
            if (self)
                self->tryScreenSaver(reason);
        });
}

// Fallback for desktops without a usable Inhibit portal. Not reachable from a
// Flatpak unless the manifest grants access to this bus name.
void ScreenInhibitor::tryScreenSaver(const QString& reason)
{
    QString appName = QCoreApplication::applicationName();
    if (appName.isEmpty())
        appName = QStringLiteral("Collabora Office");

    QDBusMessage message = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.ScreenSaver"),
        QStringLiteral("/org/freedesktop/ScreenSaver"),
        QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("Inhibit"));
    message.setArguments(QVariantList{ appName, reason });

    auto* watcher = new QDBusPendingCallWatcher(
        QDBusConnection::sessionBus().asyncCall(message, DBUS_TIMEOUT_MS));

    // Null once this object is destroyed, which tells the reply handler below
    // that nobody wants the inhibit any more.
    QPointer<ScreenInhibitor> self(this);
    QObject::connect(
        watcher, &QDBusPendingCallWatcher::finished, watcher,
        [self](QDBusPendingCallWatcher* call)
        {
            call->deleteLater();
            QDBusPendingReply<unsigned int> reply = *call;

            if (reply.isValid())
            {
                const unsigned int cookie = reply.value();
                if (self)
                {
                    self->_screenSaverCookie = cookie;
                    self->_backend = Backend::ScreenSaver;
                    LOG_INF("Inhibiting idle through org.freedesktop.ScreenSaver, cookie "
                            << cookie);
                }
                else
                {
                    unInhibitScreenSaver(cookie);
                }
                return;
            }

            LOG_WRN("Could not inhibit the screensaver (the slideshow still works): neither the "
                    "Inhibit portal nor org.freedesktop.ScreenSaver is available: "
                    << reply.error().message().toStdString());
            if (self)
                self->_backend = Backend::None;
        });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
