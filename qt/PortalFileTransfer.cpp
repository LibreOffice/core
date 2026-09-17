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

#include "PortalFileTransfer.hpp"

#include <common/Log.hpp>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QMimeData>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace
{
constexpr const char* PORTAL_FILETRANSFER_MIME = "application/vnd.portal.filetransfer";
}

bool hasPortalFileTransfer(const QMimeData* mimeData)
{
    return mimeData && mimeData->hasFormat(PORTAL_FILETRANSFER_MIME);
}

QStringList filesFromPortalTransfer(const QMimeData* mimeData)
{
    if (!hasPortalFileTransfer(mimeData))
        return {};

    // The payload is the transfer key, it can contain a trailing NUL.
    const QString key =
        QString::fromUtf8(mimeData->data(PORTAL_FILETRANSFER_MIME)).remove(QChar('\0'));
    if (key.isEmpty())
        return {};

    QDBusMessage message = QDBusMessage::createMethodCall(
        "org.freedesktop.portal.Documents", "/org/freedesktop/portal/documents",
        "org.freedesktop.portal.FileTransfer", "RetrieveFiles");
    message.setArguments(QVariantList{ key, QVariantMap{} });

    QDBusReply<QStringList> reply = QDBusConnection::sessionBus().call(message);
    if (!reply.isValid())
    {
        LOG_WRN(
            "FileTransfer portal RetrieveFiles failed: " << reply.error().message().toStdString());
        return {};
    }
    return reply.value();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
