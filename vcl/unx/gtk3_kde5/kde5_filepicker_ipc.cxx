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

#include "kde5_filepicker_ipc.hxx"

#include <QSocketNotifier>
#include <QUrl>
#include <QThread>
#include <QApplication>
#include <QDebug>

#include <iostream>

#include "filepicker_ipc_commands.hxx"
#include "kde5_filepicker.hxx"

#include <rtl/ustring.h>

void readIpcArg(std::istream& stream, QString& string)
{
    const auto buffer = readIpcStringArg(stream);
    string = QString::fromUtf8(buffer.data(), buffer.size());
}

void sendIpcArg(std::ostream& stream, const QString& string)
{
    const auto utf8 = string.toUtf8();
    sendIpcStringArg(stream, utf8.size(), utf8.data());
}

void sendIpcArg(std::ostream& stream, const QList<QUrl>& urls)
{
    stream << static_cast<uint32_t>(urls.size()) << ' ';
    for (const auto& url : urls)
    {
        sendIpcArg(stream, url.toString());
    }
}

FilePickerIpc::FilePickerIpc(KDE5FilePicker* filePicker, QObject* parent)
    : QObject(parent)
    , m_filePicker(filePicker)
    , m_stdinNotifier(new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, this))
{
    connect(m_stdinNotifier, &QSocketNotifier::activated, this, &FilePickerIpc::readCommands);
}

FilePickerIpc::~FilePickerIpc() = default;

void FilePickerIpc::readCommands()
{
    while (!std::cin.eof())
    {
        readCommand();
    }
}

void FilePickerIpc::readCommand()
{
    uint64_t messageId = 0;
    Commands command;
    readIpcArgs(std::cin, messageId, command);

    switch (command)
    {
        case Commands::SetTitle:
        {
            QString title;
            readIpcArgs(std::cin, title);
            m_filePicker->setTitle(title);
            return;
        }
        case Commands::SetWinId:
        {
            sal_uIntPtr winId = 0;
            readIpcArgs(std::cin, winId);
            m_filePicker->setWinId(winId);
            return;
        }
        case Commands::Execute:
        {
            sendIpcArgs(std::cout, messageId, m_filePicker->execute());
            return;
        }
        case Commands::SetMultiSelectionMode:
        {
            bool multiSelection = false;
            readIpcArgs(std::cin, multiSelection);
            m_filePicker->setMultiSelectionMode(multiSelection);
            return;
        }
        case Commands::SetDefaultName:
        {
            QString name;
            readIpcArgs(std::cin, name);
            m_filePicker->setDefaultName(name);
            return;
        }
        case Commands::SetDisplayDirectory:
        {
            QString dir;
            readIpcArgs(std::cin, dir);
            m_filePicker->setDisplayDirectory(dir);
            return;
        }
        case Commands::GetDisplayDirectory:
        {
            sendIpcArgs(std::cout, messageId, m_filePicker->getDisplayDirectory());
            return;
        }
        case Commands::GetSelectedFiles:
        {
            sendIpcArgs(std::cout, messageId, m_filePicker->getSelectedFiles());
            return;
        }
        case Commands::AppendFilter:
        {
            QString title, filter;
            readIpcArgs(std::cin, title, filter);
            m_filePicker->appendFilter(title, filter);
            return;
        }
        case Commands::SetCurrentFilter:
        {
            QString title;
            readIpcArgs(std::cin, title);
            m_filePicker->setCurrentFilter(title);
            return;
        }
        case Commands::GetCurrentFilter:
        {
            sendIpcArgs(std::cout, messageId, m_filePicker->getCurrentFilter());
            return;
        }
        case Commands::SetValue:
        {
            sal_Int16 controlId = 0;
            sal_Int16 nControlAction = 0;
            bool value = false;
            readIpcArgs(std::cin, controlId, nControlAction, value);
            m_filePicker->setValue(controlId, nControlAction, value);
            return;
        }
        case Commands::GetValue:
        {
            sal_Int16 controlId = 0;
            sal_Int16 nControlAction = 0;
            readIpcArgs(std::cin, controlId, nControlAction);
            sendIpcArgs(std::cout, messageId, m_filePicker->getValue(controlId, nControlAction));
            return;
        }
        case Commands::EnableControl:
        {
            sal_Int16 controlId = 0;
            bool enabled = false;
            readIpcArgs(std::cin, controlId, enabled);
            m_filePicker->enableControl(controlId, enabled);
            return;
        }
        case Commands::SetLabel:
        {
            sal_Int16 controlId = 0;
            QString label;
            readIpcArgs(std::cin, controlId, label);
            m_filePicker->setLabel(controlId, label);
            return;
        }
        case Commands::GetLabel:
        {
            sal_Int16 controlId = 0;
            readIpcArgs(std::cin, controlId);
            sendIpcArgs(std::cout, messageId, m_filePicker->getLabel(controlId));
            return;
        }
        case Commands::AddCheckBox:
        {
            sal_Int16 controlId = 0;
            bool hidden = false;
            QString label;
            readIpcArgs(std::cin, controlId, hidden, label);
            m_filePicker->addCheckBox(controlId, label, hidden);
            return;
        }
        case Commands::Initialize:
        {
            bool saveDialog = false;
            readIpcArgs(std::cin, saveDialog);
            m_filePicker->initialize(saveDialog);
            return;
        }
        case Commands::EnablePickFolderMode:
        {
            m_filePicker->enableFolderMode();
            return;
        }
        case Commands::Quit:
        {
            qApp->quit();
            return;
        }
    }
    qWarning() << "unhandled command " << static_cast<uint16_t>(command);
}

#include "kde5_filepicker_ipc.moc"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
