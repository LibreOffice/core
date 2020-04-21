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

#include "KF5FilePicker.hxx"
#include "kde5_filepicker_ipc.hxx"

#include <vcl/svapp.hxx>

#include <QApplication>
#include <QCommandLineParser>

#include <config_version.h>

int main(int argc, char** argv)
{
    QApplication::setOrganizationName("LibreOffice");
    QApplication::setOrganizationDomain("libreoffice.org");
    QApplication::setApplicationName(QStringLiteral("lo_kde5filepicker"));
    QApplication::setQuitOnLastWindowClosed(false);
    QApplication::setApplicationVersion(LIBO_VERSION_DOTTED);

    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QObject::tr("Helper executable for LibreOffice KDE/Plasma integration.\n"
                    "Do not run this executable directly. Rather, use it indirectly via "
                    "the gtk3_kde5 VCL plugin (SAL_USE_VCLPLUGIN=gtk3_kde5)."));
    parser.addVersionOption();
    parser.addHelpOption();
    QCommandLineOption folderOption("folder");
    parser.addOption(folderOption);
    parser.process(app);


    QFileDialog::FileMode fileMode = parser.isSet(folderOption) ? QFileDialog::Directory
                                                                : QFileDialog::ExistingFiles;
    std::unique_ptr<Qt5FilePicker> filePicker;
    if (Application::GetDesktopEnvironment() == "PLASMA5")
        filePicker.reset(new KF5FilePicker(fileMode));
    else
        filePicker.reset(new Qt5FilePicker(fileMode));

    FilePickerIpc ipc(filePicker.get());

    return QApplication::exec();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
