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

#include "gtk3_kde5_filepicker_ipc.hxx"

#undef Region

#include "unx/geninst.h"

#include "strings.hrc"

#include <future>

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/syswin.hxx>

#include <osl/file.h>
#include <osl/process.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <unx/gtk/gtkdata.hxx>

#include <boost/filesystem/path.hpp>
#include <boost/process/environment.hpp>
#include <boost/process/search_path.hpp>
#include <boost/process/io.hpp>

using namespace ::com::sun::star::ui::dialogs;

namespace bp = boost::process;
namespace bf = boost::filesystem;

// helper functions

namespace
{
bf::path applicationDirPath()
{
    OUString applicationFilePath;
    osl_getExecutableFile(&applicationFilePath.pData);
    OUString applicationSystemPath;
    osl_getSystemPathFromFileURL(applicationFilePath.pData, &applicationSystemPath.pData);
    auto sysPath = applicationSystemPath.toUtf8();
    auto ret = bf::path(sysPath.getStr(), sysPath.getStr() + sysPath.getLength());
    ret.remove_filename();
    return ret;
}

bf::path findPickerExecutable()
{
    auto paths = boost::this_process::path();
    paths.insert(paths.begin(), applicationDirPath());
    auto ret = bp::search_path("lo_kde5filepicker", paths);
    if (ret.empty())
        throw bp::process_error(std::make_error_code(std::errc::no_such_file_or_directory),
                                "could not find lo_kde5filepicker executable");
    return ret;
}
}

void readIpcArg(std::istream& stream, OUString& str)
{
    const auto buffer = readIpcStringArg(stream);
    str = OUString::fromUtf8(OString(buffer.data(), buffer.size()));
}

void readIpcArg(std::istream& stream, css::uno::Sequence<OUString>& seq)
{
    uint32_t numFiles = 0;
    stream >> numFiles;
    stream.ignore(); // skip space;
    seq.realloc(numFiles);
    for (size_t i = 0; i < numFiles; ++i)
    {
        readIpcArg(stream, seq[i]);
    }
}

void sendIpcArg(std::ostream& stream, const OUString& string)
{
    const auto utf8 = string.toUtf8();
    sendIpcStringArg(stream, utf8.getLength(), utf8.getStr());
}

OUString getResString(const char* pResId)
{
    if (pResId == nullptr)
        return {};

    return VclResId(pResId);
}

// Gtk3KDE5FilePicker

Gtk3KDE5FilePickerIpc::Gtk3KDE5FilePickerIpc()
    // workaround: specify some non-empty argument, otherwise the Qt app will see argc == 0
    : m_process(findPickerExecutable(), "dummy", bp::std_out > m_stdout, bp::std_in < m_stdin)
{
}

Gtk3KDE5FilePickerIpc::~Gtk3KDE5FilePickerIpc()
{
    sendCommand(Commands::Quit);
    if (m_process.running())
        m_process.wait_for(std::chrono::milliseconds(100));
}

sal_Int16 SAL_CALL Gtk3KDE5FilePickerIpc::execute()
{
    auto restoreMainWindow = blockMainWindow();

    auto id = sendCommand(Commands::Execute);
    sal_Bool accepted = false;
    readResponse(id, accepted);

    if (restoreMainWindow)
        restoreMainWindow();

    return accepted ? ExecutableDialogResults::OK : ExecutableDialogResults::CANCEL;
}

static gboolean ignoreDeleteEvent(GtkWidget* /*widget*/, GdkEvent* /*event*/,
                                  gpointer /*user_data*/)
{
    return true;
}

std::function<void()> SAL_CALL Gtk3KDE5FilePickerIpc::blockMainWindow()
{
    vcl::Window* pParentWin = Application::GetDefDialogParent();
    if (!pParentWin)
        return {};

    const SystemEnvData* pSysData = static_cast<SystemWindow*>(pParentWin)->GetSystemData();
    if (!pSysData)
        return {};

    sendCommand(Commands::SetWinId, pSysData->aWindow);

    auto* pMainWindow = reinterpret_cast<GtkWidget*>(pSysData->pWidget);
    if (!pMainWindow)
        return {};

    SolarMutexGuard guard;
    auto deleteEventSignalId = g_signal_lookup("delete_event", gtk_widget_get_type());

    // disable the mainwindow
    gtk_widget_set_sensitive(pMainWindow, false);

    // block the GtkSalFrame delete_event handler
    auto blockedHandler = g_signal_handler_find(
        pMainWindow, static_cast<GSignalMatchType>(G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA),
        deleteEventSignalId, 0, nullptr, nullptr, pSysData->pSalFrame);
    g_signal_handler_block(pMainWindow, blockedHandler);

    // prevent the window from being closed
    auto ignoreDeleteEventHandler
        = g_signal_connect(pMainWindow, "delete_event", G_CALLBACK(ignoreDeleteEvent), nullptr);

    return [pMainWindow, ignoreDeleteEventHandler, blockedHandler] {
        SolarMutexGuard cleanupGuard;
        // re-enable window
        gtk_widget_set_sensitive(pMainWindow, true);

        // allow it to be closed again
        g_signal_handler_disconnect(pMainWindow, ignoreDeleteEventHandler);

        // unblock the GtkSalFrame handler
        g_signal_handler_unblock(pMainWindow, blockedHandler);
    };
}

void SAL_CALL Gtk3KDE5FilePickerIpc::await(const std::future<void>& future)
{
    while (future.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready)
    {
        GetGtkSalData()->Yield(false, true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
