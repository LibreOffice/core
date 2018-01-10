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

#include "gtk3_kde5_folderpicker.hxx"

#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <vcl/svapp.hxx>

#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

// constructor

Gtk3KDE5FolderPicker::Gtk3KDE5FolderPicker(
    const uno::Reference<uno::XComponentContext>& /*xContext*/)
{
    m_ipc.sendCommand(Commands::EnablePickFolderMode);
    setTitle(getResString(STR_FPICKER_FOLDER_DEFAULT_TITLE));
}

Gtk3KDE5FolderPicker::~Gtk3KDE5FolderPicker() = default;

void SAL_CALL Gtk3KDE5FolderPicker::setDisplayDirectory(const OUString& aDirectory)
{
    m_ipc.sendCommand(Commands::SetDisplayDirectory, aDirectory);
}

OUString SAL_CALL Gtk3KDE5FolderPicker::getDisplayDirectory()
{
    auto id = m_ipc.sendCommand(Commands::GetDisplayDirectory);
    OUString ret;
    m_ipc.readResponse(id, ret);
    return ret;
}

OUString SAL_CALL Gtk3KDE5FolderPicker::getDirectory()
{
    auto id = m_ipc.sendCommand(Commands::GetSelectedFiles);
    uno::Sequence<OUString> seq;
    m_ipc.readResponse(id, seq);
    return seq.hasElements() ? seq[0] : OUString();
}

void SAL_CALL Gtk3KDE5FolderPicker::setDescription(const OUString& /*rDescription*/) {}

// XExecutableDialog functions

void SAL_CALL Gtk3KDE5FolderPicker::setTitle(const OUString& aTitle)
{
    m_ipc.sendCommand(Commands::SetTitle, aTitle);
}

sal_Int16 SAL_CALL Gtk3KDE5FolderPicker::execute() { return m_ipc.execute(); }

// XCancellable

void SAL_CALL Gtk3KDE5FolderPicker::cancel()
{
    // TODO
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
