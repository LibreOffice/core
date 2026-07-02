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

#include "DBusService.hpp"
#include <qt/WebView.hpp>
#include <common/Log.hpp>
#include <COKit/COKit.h>
#include <qt/qt.hpp>
#include <Poco/URI.h>
#include <Poco/Path.h>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QFile>
#include <QFileInfo>
#include <sys/xattr.h>

constexpr const char* SERVICE_NAME = "com.collaboraoffice.Office";
constexpr const char* OBJECT_PATH = "/com/collaboraoffice/Office";

namespace coda
{
    std::string hostDisplayUriForPath(const QString& path)
    {
        // The xdg-document-portal FUSE filesystem exposes the real host location
        // of an exported file through this extended attribute.
        constexpr const char* HOST_PATH_XATTR = "user.document-portal.host-path";

        const QByteArray local = QFile::encodeName(path);

        const ssize_t needed = getxattr(local.constData(), HOST_PATH_XATTR, nullptr, 0);
        if (needed <= 0)
            return {};

        QByteArray buf(needed, Qt::Uninitialized);
        const ssize_t len = getxattr(local.constData(), HOST_PATH_XATTR, buf.data(), buf.size());
        if (len <= 0)
            return {};
        buf.resize(len);

        const QString hostPath = QFile::decodeName(buf);
        if (hostPath.isEmpty())
            return {};

        return Poco::URI(Poco::Path(hostPath.toStdString())).toString();
    }

    void openFiles(const QStringList& files, const QStringList& displayUris)
    {
        for (int i = 0; i < files.size(); ++i)
        {
            const QString& file = files[i];
            Poco::URI fileURL(Poco::Path(file.toStdString()));

            // if document is already open, just activate it
            WebView* existingDocument = WebView::findOpenDocument(fileURL);
            if (existingDocument)
            {
                existingDocument->activateWindow();
                continue;
            }

            WebView* webViewInstance = new WebView(Application::getProfile());
            webViewInstance->load(fileURL);

            QFileInfo fileInfo(file);
            Poco::URI uri(Poco::Path(fileInfo.absoluteFilePath().toStdString()));
            const QString displayUri = i < displayUris.size() ? displayUris[i] : QString();
            Application::getRecentFiles().add(uri.toString(), displayUri.toStdString());
        }
    }

    void openNewDocument(const QString& templateType)
    {
        WebView* webViewInstance = WebView::createNewDocument(Application::getProfile(), templateType.toStdString(), {}, {});
        if (!webViewInstance)
        {
            LOG_ERR("Failed to create new document");
        }
    }
}

DBusService::DBusService(QObject* parent)
    : QObject(parent)
{
}

DBusService::~DBusService()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    // Unregister the object first
    sessionBus.unregisterObject(OBJECT_PATH);

    // Then unregister the service name
    sessionBus.unregisterService(SERVICE_NAME);
}

void DBusService::openFiles(const QStringList& files)
{
    coda::openFiles(files);
}

void DBusService::openNewDocument(const QString& templateType)
{
    coda::openNewDocument(templateType);
}

void DBusService::activate()
{
    if (auto instance = WebView::getAllInstances().front())
        instance->activateWindow();
}

bool DBusService::tryForwardToExistingInstance(const QStringList& files, const QString& templateType)
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    if (!sessionBus.interface()->isServiceRegistered(SERVICE_NAME))
    {
        // no existing instance
        return false;
    }

    QDBusMessage dbusMessage;
    if (!files.isEmpty())
    {
        dbusMessage = QDBusMessage::createMethodCall(SERVICE_NAME, OBJECT_PATH, SERVICE_NAME, "openFiles");
        dbusMessage.setArguments(QVariantList{ files });
    }
    else if (!templateType.isEmpty())
    {
        dbusMessage = QDBusMessage::createMethodCall(SERVICE_NAME, OBJECT_PATH, SERVICE_NAME, "openNewDocument");
        dbusMessage.setArguments(QVariantList{ templateType });
    }
    else
    {
        dbusMessage = QDBusMessage::createMethodCall(SERVICE_NAME, OBJECT_PATH, SERVICE_NAME, "activate");
    }

    QDBusReply<void> reply = sessionBus.call(dbusMessage);
    if (!reply.isValid())
    {
        LOG_ERR("DBus call failed: " << reply.error().message().toStdString());
        return false;
    }

    return true;
}

bool DBusService::registerService(DBusService* service)
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    if (!sessionBus.registerService(SERVICE_NAME))
    {
        LOG_ERR("Failed to register DBus service: " << sessionBus.lastError().message().toStdString());
        return false;
    }

    if (!sessionBus.registerObject(OBJECT_PATH, service, QDBusConnection::ExportAllSlots))
    {
        LOG_ERR("Failed to register DBus object: " << sessionBus.lastError().message().toStdString());
        return false;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

