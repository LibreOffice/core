/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Fuzzer for ClientRequestDispatcher HTTP request handling.
 * Functions: LLVMFuzzerTestOneInput() - Tests ClientRequestDispatcher
 */

#include <config.h>

#include <common/SigUtil.hpp>
#include <fuzzer/Common.hpp>
#include <net/HttpRequest.hpp>
#include <test/MockStreamSocket.hpp>
#include <wsd/Admin.hpp>
#include <wsd/ClientRequestDispatcher.hpp>
#include <wsd/ContentType.hpp>

#include <cstdint>

extern "C" int LLVMFuzzerInitialize(int* /*argc*/, char*** /*argv*/)
{
    fuzzer::DoInitialization();
    Admin::initialize();

    std::atexit(
        []
        {
            SigUtil::setTerminationFlag();

            Admin::uninitialize();
        });

    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    try
    {
        ClientRequestDispatcher::uninitialize(); // Clear statics.

        std::shared_ptr<ProtocolHandlerInterface> handler =
            std::make_shared<ClientRequestDispatcher>();

        auto socket = std::make_shared<MockStreamSocket>();
        socket->setHandler(handler);
        handler->onConnect(socket);
        Buffer& inBuf = socket->getInBuffer();
        inBuf.append(reinterpret_cast<const char*>(data), size);

        size_t lastSize = size;
        for (;;)
        {
            SocketDisposition disposition(socket);
            handler->handleIncomingMessage(disposition);
            if (disposition.isTransfer())
            {
                disposition.execute(); // In case we have to move, to clear it.
                break; // We can't reuse this socket.
            }

            if (disposition.isClosed() || socket->isShutdownSignalled() || lastSize == inBuf.size())
                break; // We can't reuse this socket.

            lastSize = inBuf.size();
        }

        // Avoid destruction-time checks as we can hold the last reference,
        // after the thread ownership had moved, which will assert.
        socket->resetThreadOwner();
    }
    catch (const std::exception&)
    {
        // Bad Request, etc.
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
