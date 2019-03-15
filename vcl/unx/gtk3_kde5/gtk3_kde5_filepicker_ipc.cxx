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

#include <unx/geninst.h>

#include <strings.hrc>

#include <future>
#include <system_error>

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

using namespace ::com::sun::star::ui::dialogs;

// helper functions

namespace
{
OUString applicationDirPath()
{
    OUString applicationFilePath;
    osl_getExecutableFile(&applicationFilePath.pData);
    OUString applicationSystemPath;
    osl_getSystemPathFromFileURL(applicationFilePath.pData, &applicationSystemPath.pData);
    const auto utf8Path = applicationSystemPath.toUtf8();
    auto ret = boost::filesystem::path(utf8Path.getStr(), utf8Path.getStr() + utf8Path.getLength());
    ret.remove_filename();
    return OUString::fromUtf8(OString(ret.c_str(), strlen(ret.c_str())));
}

OUString findPickerExecutable()
{
    const auto path = applicationDirPath();
    const OUString app("lo_kde5filepicker");
    OUString ret;
    osl_searchFileURL(app.pData, path.pData, &ret.pData);
    if (ret.isEmpty())
        throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory),
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

// handles the IPC commands for dialog execution and ends the dummy Gtk dialog once the IPC response is there
static void handleIpcForExecute(Gtk3KDE5FilePickerIpc* pFilePickerIpc, GtkWidget* pDummyDialog,
                                bool* bResult)
{
    auto id = pFilePickerIpc->sendCommand(Commands::Execute);
    pFilePickerIpc->readResponse(id, *bResult);

    // end the dummy dialog
    gtk_widget_hide(pDummyDialog);
}

// Gtk3KDE5FilePicker

Gtk3KDE5FilePickerIpc::Gtk3KDE5FilePickerIpc()
{
    const auto exe = findPickerExecutable();
    oslProcessError result;
    oslSecurity pSecurity = osl_getCurrentSecurity();
    result = osl_executeProcess_WithRedirectedIO(exe.pData, nullptr, 0, osl_Process_NORMAL,
                                                 pSecurity, nullptr, nullptr, 0, &m_process,
                                                 &m_inputWrite, &m_outputRead, nullptr);
    osl_freeSecurityHandle(pSecurity);
    if (result != osl_Process_E_None)
        throw std::system_error(std::make_error_code(std::errc::no_such_process),
                                "could not start lo_kde5filepicker executable");
}

Gtk3KDE5FilePickerIpc::~Gtk3KDE5FilePickerIpc()
{
    if (!m_process)
        return;

    sendCommand(Commands::Quit);
    TimeValue timeValue(std::chrono::milliseconds(100));
    if (osl_joinProcessWithTimeout(m_process, &timeValue) != osl_Process_E_None)
        osl_terminateProcess(m_process);

    if (m_inputWrite)
        osl_closeFile(m_inputWrite);
    if (m_outputRead)
        osl_closeFile(m_outputRead);
    osl_freeProcessHandle(m_process);
}

sal_Int16 Gtk3KDE5FilePickerIpc::execute()
{
    auto restoreMainWindow = blockMainWindow();

    // dummy gtk dialog that will take care of processing events,
    // not meant to be actually seen by user
    GtkWidget* pDummyDialog = gtk_dialog_new();

    bool accepted = false;

    // send IPC command and read response in a separate thread
    std::thread aIpcHandler(&handleIpcForExecute, this, pDummyDialog, &accepted);

    // make dummy dialog not to be seen by user
    gtk_window_set_decorated(GTK_WINDOW(pDummyDialog), false);
    gtk_window_set_default_size(GTK_WINDOW(pDummyDialog), 0, 0);
    gtk_window_set_accept_focus(GTK_WINDOW(pDummyDialog), false);
    // gtk_widget_set_opacity() only has the desired effect when widget is already shown
    gtk_widget_show(pDummyDialog);
    gtk_widget_set_opacity(pDummyDialog, 0);
    // run dialog, leaving event processing to GTK
    // dialog will be closed by the separate 'aIpcHandler' thread once the IPC response is there
    gtk_dialog_run(GTK_DIALOG(pDummyDialog));

    aIpcHandler.join();

    gtk_widget_destroy(pDummyDialog);

    if (restoreMainWindow)
        restoreMainWindow();

    return accepted ? ExecutableDialogResults::OK : ExecutableDialogResults::CANCEL;
}

static gboolean ignoreDeleteEvent(GtkWidget* /*widget*/, GdkEvent* /*event*/,
                                  gpointer /*user_data*/)
{
    return true;
}

std::function<void()> Gtk3KDE5FilePickerIpc::blockMainWindow()
{
    vcl::Window* pParentWin = Application::GetDefDialogParent();
    if (!pParentWin)
        return {};

    const SystemEnvData* pSysData = static_cast<SystemWindow*>(pParentWin)->GetSystemData();
    if (!pSysData)
        return {};

    sendCommand(Commands::SetWinId, pSysData->aWindow);

    auto* pMainWindow = static_cast<GtkWidget*>(pSysData->pWidget);
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

void Gtk3KDE5FilePickerIpc::writeResponseLine(const std::string& line)
{
    sal_uInt64 bytesWritten = 0;
    osl_writeFile(m_inputWrite, line.c_str(), line.size(), &bytesWritten);
}

std::string Gtk3KDE5FilePickerIpc::readResponseLine()
{
    if (!m_responseBuffer.empty()) // check whether we have a line in our buffer
    {
        std::size_t it = m_responseBuffer.find('\n');
        if (it != std::string::npos)
        {
            auto ret = m_responseBuffer.substr(0, it);
            m_responseBuffer.erase(0, it + 1);
            return ret;
        }
    }

    const sal_uInt64 BUF_SIZE = 1024;
    char buffer[BUF_SIZE];
    while (true)
    {
        sal_uInt64 bytesRead = 0;
        auto err = osl_readFile(m_outputRead, buffer, BUF_SIZE, &bytesRead);
        auto it = std::find(buffer, buffer + bytesRead, '\n');
        if (it != buffer + bytesRead) // check whether the chunk we read contains an EOL
        {
            // if so, append that part to the buffer and return it
            std::string ret = m_responseBuffer.append(buffer, it);
            // but keep anything else we may have read in our buffer
            ++it;
            m_responseBuffer.assign(it, buffer + bytesRead);
            return ret;
        }
        // otherwise append everything we read to the buffer and try again
        m_responseBuffer.append(buffer, bytesRead);

        if (err != osl_File_E_None && err != osl_File_E_AGAIN)
            break;
    }
    return {};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
