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

#include <config.h>

#include <common/Log.hpp>
#include <common/Util.hpp>
#include <net/Socket.hpp>

// Every Windows binary here is an app build, where the descriptors are fake sockets, so none of
// these is reached.

namespace net
{

void shutdownDescriptor(int)
{
    LOG_FTL("Shutting down a real socket descriptor is not implemented for this platform.");
    Util::forcedExit(EX_SOFTWARE);
}

void closeDescriptor(int)
{
    LOG_FTL("Closing a real descriptor is not implemented for this platform.");
    Util::forcedExit(EX_SOFTWARE);
}

ssize_t writeDescriptor(int, const void*, std::size_t)
{
    LOG_FTL("Writing to a real descriptor is not implemented for this platform.");
    Util::forcedExit(EX_SOFTWARE);
}

void disableNagleAlgorithm(int)
{
    LOG_FTL("Setting TCP_NODELAY on a real socket descriptor is not implemented for this "
            "platform.");
    Util::forcedExit(EX_SOFTWARE);
}

ssize_t readDescriptor(int, void*, std::size_t)
{
    LOG_FTL("Reading from a real descriptor is not implemented for this platform.");
    Util::forcedExit(EX_SOFTWARE);
}

int createPipe(int[2])
{
    LOG_FTL("Creating a real pipe is not implemented for this platform.");
    Util::forcedExit(EX_SOFTWARE);
}

int pollDescriptors(struct pollfd*, std::size_t, int64_t)
{
    LOG_FTL("Waiting on real descriptors is not implemented for this platform.");
    Util::forcedExit(EX_SOFTWARE);
}

int openStreamSocket(Socket::Type)
{
    LOG_FTL("Opening a real stream socket is not implemented for this platform.");
    Util::forcedExit(EX_SOFTWARE);
}

bool bindToPort(int, Socket::Type, bool, int)
{
    LOG_FTL("Binding a real socket to a port is not implemented for this platform.");
    Util::forcedExit(EX_SOFTWARE);
}

bool isUnrecoverableAcceptError(int)
{
    LOG_FTL("Accepting on a real listening socket is not implemented for this platform.");
    Util::forcedExit(EX_SOFTWARE);
}

int acceptConnection(int, PeerAddress&)
{
    LOG_FTL("Accepting on a real listening socket is not implemented for this platform.");
    Util::forcedExit(EX_SOFTWARE);
}

} // namespace net

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
