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

void readCommandArgs(Commands command, QList<QVariant>& args)
{
    switch (command)
    {
        case Commands::SetTitle:
        {
            QString title;
            readIpcArgs(std::cin, title);
            args.append(title);
            break;
        }
        case Commands::SetWinId:
        {
            sal_uIntPtr winId = 0;
            readIpcArgs(std::cin, winId);
            QVariant aWinIdVariant;
            aWinIdVariant.setValue(winId);
            args.append(aWinIdVariant);
            break;
        }
        case Commands::SetMultiSelectionMode:
        {
            bool multiSelection = false;
            readIpcArgs(std::cin, multiSelection);
            args.append(multiSelection);
            break;
        }
        case Commands::SetDefaultName:
        {
            QString name;
            readIpcArgs(std::cin, name);
            args.append(name);
            break;
        }
        case Commands::SetDisplayDirectory:
        {
            QString dir;
            readIpcArgs(std::cin, dir);
            args.append(dir);
            break;
        }
        case Commands::AppendFilter:
        {
            QString title, filter;
            readIpcArgs(std::cin, title, filter);
            args.append(title);
            args.append(filter);
            break;
        }
        case Commands::SetCurrentFilter:
        {
            QString title;
            readIpcArgs(std::cin, title);
            args.append(title);
            break;
        }
        case Commands::SetValue:
        {
            sal_Int16 controlId = 0;
            sal_Int16 nControlAction = 0;
            bool value = false;
            readIpcArgs(std::cin, controlId, nControlAction, value);
            args.append(controlId);
            args.append(nControlAction);
            args.append(value);
            break;
        }
        case Commands::GetValue:
        {
            sal_Int16 controlId = 0;
            sal_Int16 nControlAction = 0;
            readIpcArgs(std::cin, controlId, nControlAction);
            args.append(controlId);
            args.append(nControlAction);
            break;
        }
        case Commands::EnableControl:
        {
            sal_Int16 controlId = 0;
            bool enabled = false;
            readIpcArgs(std::cin, controlId, enabled);
            args.append(controlId);
            args.append(enabled);
            break;
        }
        case Commands::SetLabel:
        {
            sal_Int16 controlId = 0;
            QString label;
            readIpcArgs(std::cin, controlId, label);
            args.append(controlId);
            args.append(label);
            break;
        }
        case Commands::GetLabel:
        {
            sal_Int16 controlId = 0;
            readIpcArgs(std::cin, controlId);
            args.append(controlId);
            break;
        }
        case Commands::AddCheckBox:
        {
            sal_Int16 controlId = 0;
            bool hidden = false;
            QString label;
            readIpcArgs(std::cin, controlId, hidden, label);
            args.append(controlId);
            args.append(hidden);
            args.append(label);
            break;
        }
        case Commands::Initialize:
        {
            bool saveDialog = false;
            readIpcArgs(std::cin, saveDialog);
            args.append(saveDialog);
            break;
        }
        default:
        {
            // no extra parameters/arguments
            break;
        }
    };
}

void readCommands(FilePickerIpc* ipc)
{
    while (!std::cin.eof())
    {
        uint64_t messageId = 0;
        Commands command;
        readIpcArgs(std::cin, messageId, command);

        // retrieve additional command-specific arguments
        QList<QVariant> args;
        readCommandArgs(command, args);

        emit ipc->commandReceived(messageId, command, args);

        // stop processing once 'Quit' command has been sent
        if (command == Commands::Quit)
        {
            return;
        }
    }
}

FilePickerIpc::FilePickerIpc(KDE5FilePicker* filePicker, QObject* _parent)
    : QObject(_parent)
    , m_filePicker(filePicker)
{
    // required to be able to pass those via signal/slot
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<Commands>("Commands");

    connect(this, &FilePickerIpc::commandReceived, this, &FilePickerIpc::handleCommand);

    // read IPC commands and their args in a separate thread, so this does not block everything else;
    // 'commandReceived' signal is emitted every time a command and its args have been read;
    // thread will run until the filepicker process is terminated
    m_ipcReaderThread = std::unique_ptr<std::thread>{ new std::thread(readCommands, this) };
}

FilePickerIpc::~FilePickerIpc()
{
    // join thread that reads commands
    m_ipcReaderThread->join();
};

bool FilePickerIpc::handleCommand(uint64_t messageId, Commands command, QList<QVariant> args)
{
    switch (command)
    {
        case Commands::SetTitle:
        {
            QString title = args.takeFirst().toString();
            m_filePicker->setTitle(title);
            return true;
        }
        case Commands::SetWinId:
        {
            sal_uIntPtr winId = args.takeFirst().value<sal_uIntPtr>();
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
            bool multiSelection = args.takeFirst().toBool();
            m_filePicker->setMultiSelectionMode(multiSelection);
            return true;
        }
        case Commands::SetDefaultName:
        {
            QString name = args.takeFirst().toString();
            m_filePicker->setDefaultName(name);
            return true;
        }
        case Commands::SetDisplayDirectory:
        {
            QString dir = args.takeFirst().toString();
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
            QString title = args.takeFirst().toString();
            QString filter = args.takeFirst().toString();
            m_filePicker->appendFilter(title, filter);
            return true;
        }
        case Commands::SetCurrentFilter:
        {
            QString title = args.takeFirst().toString();
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
            sal_Int16 controlId = args.takeFirst().value<sal_Int16>();
            sal_Int16 nControlAction = args.takeFirst().value<sal_Int16>();
            bool value = args.takeFirst().toBool();
            m_filePicker->setValue(controlId, nControlAction, value);
            return true;
        }
        case Commands::GetValue:
        {
            sal_Int16 controlId = args.takeFirst().value<sal_Int16>();
            sal_Int16 nControlAction = args.takeFirst().value<sal_Int16>();
            sendIpcArgs(std::cout, messageId, m_filePicker->getValue(controlId, nControlAction));
            return true;
        }
        case Commands::EnableControl:
        {
            sal_Int16 controlId = args.takeFirst().value<sal_Int16>();
            bool enabled = args.takeFirst().toBool();
            m_filePicker->enableControl(controlId, enabled);
            return true;
        }
        case Commands::SetLabel:
        {
            sal_Int16 controlId = args.takeFirst().value<sal_Int16>();
            QString label = args.takeFirst().toString();
            m_filePicker->setLabel(controlId, label);
            return true;
        }
        case Commands::GetLabel:
        {
            sal_Int16 controlId = args.takeFirst().value<sal_Int16>();
            sendIpcArgs(std::cout, messageId, m_filePicker->getLabel(controlId));
            return true;
        }
        case Commands::AddCheckBox:
        {
            sal_Int16 controlId = args.takeFirst().value<sal_Int16>();
            bool hidden = args.takeFirst().toBool();
            QString label = args.takeFirst().toString();
            m_filePicker->addCheckBox(controlId, label, hidden);
            return true;
        }
        case Commands::Initialize:
        {
            bool saveDialog = args.takeFirst().toBool();
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
