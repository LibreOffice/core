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

#include <cppuhelper/compbase.hxx>

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>

#include <rtl/ustrbuf.hxx>

#include <boost/process/child.hpp>
#include <boost/process/pipe.hpp>

#include "filepicker_ipc_commands.hxx"

#include <functional>
#include <future>
#include <mutex>
#include <thread>

OUString getResString(const char* pResId);

class Gtk3KDE5FilePickerIpc
{
protected:
    boost::process::ipstream m_stdout;
    boost::process::opstream m_stdin;
    boost::process::child m_process;
    // simple multiplexing: every command gets it's own ID that can be used to
    // read the corresponding response
    uint64_t m_msgId = 1;
    std::mutex m_mutex;
    uint64_t m_incomingResponse = 0;

public:
    explicit Gtk3KDE5FilePickerIpc();
    ~Gtk3KDE5FilePickerIpc();

    sal_Int16 execute();

    template <typename... Args> uint64_t sendCommand(Commands command, const Args&... args)
    {
        auto id = m_msgId;
        ++m_msgId;
        sendIpcArgs(m_stdin, id, command, args...);
        return id;
    }

    template <typename... Args> void readResponse(uint64_t id, Args&... args)
    {
        // read synchronously from a background thread and run the eventloop until the value becomes available
        // this allows us to keep the GUI responsive and also enables access to the LO clipboard
        await(std::async(std::launch::async, [&]() {
            while (true)
            {
                // only let one thread read at any given time
                std::lock_guard<std::mutex> lock(m_mutex);

                // check if we need to read (and potentially wait) a response ID
                if (m_incomingResponse == 0)
                    readIpcArgs(m_stdout, m_incomingResponse);

                if (m_incomingResponse == id)
                {
                    // the response we are waiting for came in
                    readIpcArgs(m_stdout, args...);
                    m_incomingResponse = 0;
                    break;
                }
                else
                {
                    // the next response answers some other request, yield
                    std::this_thread::yield();
                }
            }
        }));
    }

private:
    std::function<void()> blockMainWindow();

    static void await(const std::future<void>& future);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
