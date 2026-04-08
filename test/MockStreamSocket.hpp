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

#pragma once

#include <net/Socket.hpp>

#include <string>
#include <sys/eventfd.h>

/// A StreamSocket subclass for unit testing that captures output
/// without performing real I/O. Data written via send() accumulates
/// in the output buffer and can be retrieved with getOutput().
class MockStreamSocket final : public StreamSocket
{
public:
    MockStreamSocket()
        : MockStreamSocket(-1)
    {
    }

    explicit MockStreamSocket(int fd)
        : StreamSocket("localhost", fd, Socket::Type::Unix, false, LocalHost)
    {
    }

    using StreamSocket::isShutdownSignalled;
    using StreamSocket::resetThreadOwner;
    using StreamSocket::setThreadOwner;

    /// Return everything written to the output buffer as a string.
    std::string getOutput() const
    {
        const Buffer& buf = getOutBuffer();
        return std::string(buf.getBlock(), buf.getBlockSize());
    }

    /// Clear the output buffer.
    void clearOutput() { getOutBuffer().clear(); }

protected:
    /// No-op: don't write to the (invalid) fd.
    int writeOutgoingData() override { return 0; }

    /// No-op: nothing to read.
    int readIncomingData() override { return 0; }
};

/* vim:set shiftwidth=4 expandtab: */
