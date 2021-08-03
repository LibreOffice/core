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

#include <osl/process.h>
#include <unotools/resmgr.hxx>

#include "filepicker_ipc_commands.hxx"

#include <functional>
#include <mutex>
#include <thread>
#include <sstream>

OUString getResString(TranslateId pResId);

class Gtk3KDE5FilePickerIpc
{
protected:
    oslProcess m_process;
    oslFileHandle m_inputWrite;
    oslFileHandle m_outputRead;
    // simple multiplexing: every command gets its own ID that can be used to
    // read the corresponding response
    uint64_t m_msgId = 1;
    std::mutex m_mutex;
    uint64_t m_incomingResponse = 0;
    std::string m_responseBuffer;
    std::stringstream m_responseStream;

public:
    explicit Gtk3KDE5FilePickerIpc();
    ~Gtk3KDE5FilePickerIpc();

    sal_Int16 execute();

    void writeResponseLine(const std::string& line);

    template <typename... Args> uint64_t sendCommand(Commands command, const Args&... args)
    {
        auto id = m_msgId;
        ++m_msgId;
        std::stringstream stream;
        sendIpcArgs(stream, id, command, args...);
        writeResponseLine(stream.str());
        return id;
    }

    std::string readResponseLine();

    // workaround gcc <= 4.8 bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55914
    template <int...> struct seq
    {
    };
    template <int N, int... S> struct gens : gens<N - 1, N - 1, S...>
    {
    };
    template <int... S> struct gens<0, S...>
    {
        typedef seq<S...> type;
    };
    template <typename... Args> struct ArgsReader
    {
        ArgsReader(Args&... args)
            : m_args(args...)
        {
        }

        void operator()(std::istream& stream)
        {
            callFunc(stream, typename gens<sizeof...(Args)>::type());
        }

    private:
        template <int... S> void callFunc(std::istream& stream, seq<S...>)
        {
            readIpcArgs(stream, std::get<S>(m_args)...);
        }

        std::tuple<Args&...> m_args;
    };

    template <typename... Args> void readResponse(uint64_t id, Args&... args)
    {
        ArgsReader<Args...> argsReader(args...);
        while (true)
        {
            // only let one thread read at any given time
            std::scoped_lock<std::mutex> lock(m_mutex);

            // check if we need to read (and potentially wait) a response ID
            if (m_incomingResponse == 0)
            {
                m_responseStream.clear();
                m_responseStream.str(readResponseLine());
                readIpcArgs(m_responseStream, m_incomingResponse);
            }

            if (m_incomingResponse == id)
            {
                // the response we are waiting for came in
                argsReader(m_responseStream);
                m_incomingResponse = 0;
                break;
            }
            else
            {
                // the next response answers some other request, yield
                std::this_thread::yield();
            }
        }
    }

private:
    std::function<void()> blockMainWindow();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
