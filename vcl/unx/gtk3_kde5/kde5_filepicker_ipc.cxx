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

#include <QUrl>
#include <QApplication>
#include <QDebug>

#include <iostream>

#include "Qt5FilePicker.hxx"

Q_DECLARE_METATYPE(OUString)

static void readCommandArgs(Commands command, QList<QVariant>& args)
{
    switch (command)
    {
        case Commands::SetTitle:
        {
            OUString title;
            readIpcArgs(std::cin, title);
            args.append(QVariant::fromValue(title));
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
            OUString name;
            readIpcArgs(std::cin, name);
            args.append(QVariant::fromValue(name));
            break;
        }
        case Commands::SetDisplayDirectory:
        {
            OUString dir;
            readIpcArgs(std::cin, dir);
            args.append(QVariant::fromValue(dir));
            break;
        }
        case Commands::AppendFilter:
        {
            OUString title, filter;
            readIpcArgs(std::cin, title, filter);
            args.append(QVariant::fromValue(title));
            args.append(QVariant::fromValue(filter));
            break;
        }
        case Commands::SetCurrentFilter:
        {
            OUString title;
            readIpcArgs(std::cin, title);
            args.append(QVariant::fromValue(title));
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
            OUString label;
            readIpcArgs(std::cin, controlId, label);
            args.append(controlId);
            args.append(QVariant::fromValue(label));
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
            OUString label;
            readIpcArgs(std::cin, controlId, label);
            args.append(controlId);
            args.append(QVariant::fromValue(label));
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

static void readCommands(FilePickerIpc* ipc)
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

FilePickerIpc::FilePickerIpc(Qt5FilePicker* filePicker, QObject* parent)
    : QObject(parent)
    , m_filePicker(filePicker)
{
    // required to be able to pass those via signal/slot
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<Commands>("Commands");
    qRegisterMetaType<OUString>();

    connect(this, &FilePickerIpc::commandReceived, this, &FilePickerIpc::handleCommand);

    // read IPC commands and their args in a separate thread, so this does not block everything else;
    // 'commandReceived' signal is emitted every time a command and its args have been read;
    // thread will run until the filepicker process is terminated
    m_ipcReaderThread = std::make_unique<std::thread>(readCommands, this);
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
            OUString title = args.takeFirst().value<OUString>();
            m_filePicker->setTitle(title);
            return true;
        }
        case Commands::SetWinId:
        {
            sal_uIntPtr winId = args.takeFirst().value<sal_uIntPtr>();
            m_filePicker->setParentWinId(winId);
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
            OUString name = args.takeFirst().value<OUString>();
            m_filePicker->setDefaultName(name);
            return true;
        }
        case Commands::SetDisplayDirectory:
        {
            OUString dir = args.takeFirst().value<OUString>();
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
            sendIpcArgs(std::cout, messageId, m_filePicker->getSelectedFiles());
            return true;
        }
        case Commands::AppendFilter:
        {
            OUString title = args.takeFirst().value<OUString>();
            OUString filter = args.takeFirst().value<OUString>();
            m_filePicker->appendFilter(title, filter);
            return true;
        }
        case Commands::SetCurrentFilter:
        {
            OUString title = args.takeFirst().value<OUString>();
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
            m_filePicker->setValue(controlId, nControlAction, uno::Any(value));
            return true;
        }
        case Commands::GetValue:
        {
            sal_Int16 controlId = args.takeFirst().value<sal_Int16>();
            sal_Int16 nControlAction = args.takeFirst().value<sal_Int16>();
            bool ret = false;
            uno::Any value = m_filePicker->getValue(controlId, nControlAction);
            if (value.has<bool>())
                ret = value.get<bool>();
            sendIpcArgs(std::cout, messageId, ret);
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
            OUString label = args.takeFirst().value<OUString>();
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
            OUString label = args.takeFirst().value<OUString>();
            m_filePicker->addCustomControl(controlId);
            m_filePicker->setLabel(controlId, label);
            return true;
        }
        case Commands::Initialize:
        {
            bool saveDialog = args.takeFirst().toBool();
            m_filePicker->setAcceptMode(saveDialog ? QFileDialog::AcceptSave : QFileDialog::AcceptOpen);
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
