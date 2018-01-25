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

void sendIpcArg(std::ostream& stream, const QStringList& list)
{
    stream << static_cast<uint32_t>(list.size()) << ' ';
    for (const auto& entry : list)
    {
        sendIpcArg(stream, entry);
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
    while (readCommand())
    {
        // read next command
    }
}

bool FilePickerIpc::readCommand()
{
    if (std::cin.eof())
        return false;

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
            return true;
        }
        case Commands::SetWinId:
        {
            sal_uIntPtr winId = 0;
            readIpcArgs(std::cin, winId);
            m_filePicker->setWinId(winId);
            return true;
        }
        case Commands::Execute:
        {
            sendIpcArgs(std::cout, messageId, m_filePicker->execute());
            return true;
        }
        case Commands::SetMultiSelectionMode:
        {
            bool multiSelection = false;
            readIpcArgs(std::cin, multiSelection);
            m_filePicker->setMultiSelectionMode(multiSelection);
            return true;
        }
        case Commands::SetDefaultName:
        {
            QString name;
            readIpcArgs(std::cin, name);
            m_filePicker->setDefaultName(name);
            return true;
        }
        case Commands::SetDisplayDirectory:
        {
            QString dir;
            readIpcArgs(std::cin, dir);
            m_filePicker->setDisplayDirectory(dir);
            return true;
        }
        case Commands::GetDisplayDirectory:
        {
            sendIpcArgs(std::cout, messageId, m_filePicker->getDisplayDirectory());
            return true;
        }
        case Commands::GetSelectedFiles:
        {
            QStringList files;
            for (auto const& url_ : m_filePicker->getSelectedFiles())
            {
                auto url = url_;
                if (url.scheme() == QLatin1String("webdav")
                    || url.scheme() == QLatin1String("webdavs"))
                {
                    // translate webdav and webdavs URLs into a format supported by LO
                    url.setScheme(QLatin1String("vnd.sun.star.") + url.scheme());
                }
                else if (url.scheme() == QLatin1String("smb"))
                {
                    // clear the user name - the GIO backend does not support this apparently
                    // when no username is available, it will ask for the password
                    url.setUserName({});
                }
                files << url.toString();
            }
            sendIpcArgs(std::cout, messageId, files);
            return true;
        }
        case Commands::AppendFilter:
        {
            QString title, filter;
            readIpcArgs(std::cin, title, filter);
            m_filePicker->appendFilter(title, filter);
            return true;
        }
        case Commands::SetCurrentFilter:
        {
            QString title;
            readIpcArgs(std::cin, title);
            m_filePicker->setCurrentFilter(title);
            return true;
        }
        case Commands::GetCurrentFilter:
        {
            sendIpcArgs(std::cout, messageId, m_filePicker->getCurrentFilter());
            return true;
        }
        case Commands::SetValue:
        {
            sal_Int16 controlId = 0;
            sal_Int16 nControlAction = 0;
            bool value = false;
            readIpcArgs(std::cin, controlId, nControlAction, value);
            m_filePicker->setValue(controlId, nControlAction, value);
            return true;
        }
        case Commands::GetValue:
        {
            sal_Int16 controlId = 0;
            sal_Int16 nControlAction = 0;
            readIpcArgs(std::cin, controlId, nControlAction);
            sendIpcArgs(std::cout, messageId, m_filePicker->getValue(controlId, nControlAction));
            return true;
        }
        case Commands::EnableControl:
        {
            sal_Int16 controlId = 0;
            bool enabled = false;
            readIpcArgs(std::cin, controlId, enabled);
            m_filePicker->enableControl(controlId, enabled);
            return true;
        }
        case Commands::SetLabel:
        {
            sal_Int16 controlId = 0;
            QString label;
            readIpcArgs(std::cin, controlId, label);
            m_filePicker->setLabel(controlId, label);
            return true;
        }
        case Commands::GetLabel:
        {
            sal_Int16 controlId = 0;
            readIpcArgs(std::cin, controlId);
            sendIpcArgs(std::cout, messageId, m_filePicker->getLabel(controlId));
            return true;
        }
        case Commands::AddCheckBox:
        {
            sal_Int16 controlId = 0;
            bool hidden = false;
            QString label;
            readIpcArgs(std::cin, controlId, hidden, label);
            m_filePicker->addCheckBox(controlId, label, hidden);
            return true;
        }
        case Commands::Initialize:
        {
            bool saveDialog = false;
            readIpcArgs(std::cin, saveDialog);
            m_filePicker->initialize(saveDialog);
            return true;
        }
        case Commands::EnablePickFolderMode:
        {
            m_filePicker->enableFolderMode();
            return true;
        }
        case Commands::Quit:
        {
            QCoreApplication::quit();
            return false;
        }
    }
    qWarning() << "unhandled command " << static_cast<uint16_t>(command);
    QCoreApplication::exit(1);
    return false;
}

#include <kde5_filepicker_ipc.moc>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
