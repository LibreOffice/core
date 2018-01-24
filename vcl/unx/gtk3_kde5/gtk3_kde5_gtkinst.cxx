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

#include "../gtk3/gtk3gtkinst.cxx"

#include "gtk3_kde5_filepicker.hxx"
#include "gtk3_kde5_folderpicker.hxx"

#include <system_error>

uno::Reference<ui::dialogs::XFilePicker2>
GtkInstance::createFilePicker(const uno::Reference<uno::XComponentContext>& xMSF)
{
    try
    {
        return uno::Reference<ui::dialogs::XFilePicker2>(new Gtk3KDE5FilePicker(xMSF));
    }
    catch (const std::system_error& error)
    {
        OSL_FAIL(error.what());
        return { nullptr };
    }
}

uno::Reference<ui::dialogs::XFolderPicker2>
GtkInstance::createFolderPicker(const uno::Reference<uno::XComponentContext>& xMSF)
{
    try
    {
        return uno::Reference<ui::dialogs::XFolderPicker2>(new Gtk3KDE5FolderPicker(xMSF));
    }
    catch (const std::system_error& error)
    {
        OSL_FAIL(error.what());
        return { nullptr };
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
