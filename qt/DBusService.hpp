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

#include <string>

#include <QObject>
#include <QStringList>
#include <QString>

namespace coda
{
    // displayUris are used in a flatpak sandbox which contains real host locations
    void openFiles(const QStringList& files, const QStringList& displayUris = {});
    void openNewDocument(const QString& templateType);

    // Returns the host filepath for sandboxed filepaths
    std::string hostDisplayUriForPath(const QString& path);
}

class DBusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.collaboraoffice.Office")

public:
    explicit DBusService(QObject* parent = nullptr);
    ~DBusService();

    static bool tryForwardToExistingInstance(const QStringList& files, const QString& templateType);
    static bool registerService(DBusService* service);

public slots:
    void openFiles(const QStringList& files);
    void openNewDocument(const QString& templateType);
    void activate();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

