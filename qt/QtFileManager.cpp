/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <qt/QtFileManager.hpp>
#include <common/Log.hpp>

#include <QChar>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDesktopServices>
#include <QString>
#include <QStringList>
#include <QUrl>

void reveal_in_file_manager(const char* uri)
{
    if (uri == nullptr || *uri == '\0')
        return;

    const QString fileUri = QString::fromUtf8(uri);

    // Preferred: org.freedesktop.FileManager1.ShowItems opens the file manager
    // with the document revealed and selected.
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.isConnected())
    {
        QDBusMessage msg = QDBusMessage::createMethodCall(
            QStringLiteral("org.freedesktop.FileManager1"),
            QStringLiteral("/org/freedesktop/FileManager1"),
            QStringLiteral("org.freedesktop.FileManager1"),
            QStringLiteral("ShowItems"));
        msg << (QStringList() << fileUri) << QString();
        QDBusReply<void> reply = sessionBus.call(msg);
        if (reply.isValid())
            return;
        LOG_WRN("FileManager1.ShowItems failed: " << reply.error().message().toStdString()
                << "; opening the containing folder instead");
    }

    // Fallback: open the containing folder (without selecting the file).
    const QUrl folder = QUrl(fileUri).adjusted(QUrl::RemoveFilename);
    QDesktopServices::openUrl(folder);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
