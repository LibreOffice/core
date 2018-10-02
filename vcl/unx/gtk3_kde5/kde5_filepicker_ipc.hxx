/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <memory>
#include <thread>

#include <QObject>

#include "filepicker_ipc_commands.hxx"

class KDE5FilePicker;
class WinIdEmbedder;
class QSocketNotifier;

class FilePickerIpc : public QObject
{
    Q_OBJECT
public:
    explicit FilePickerIpc(KDE5FilePicker* filePicker, QObject* parent = nullptr);
    ~FilePickerIpc() override;

private:
    KDE5FilePicker* m_filePicker = nullptr;
    std::unique_ptr<std::thread> m_ipcReaderThread;

private Q_SLOTS:
    bool handleCommand(uint64_t messageId, Commands command, QList<QVariant> args);

Q_SIGNALS:
    bool commandReceived(uint64_t messageId, Commands command, QList<QVariant> args);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
