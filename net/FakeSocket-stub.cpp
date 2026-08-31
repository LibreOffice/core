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
 * The fakeSocket* family for builds that talk over real sockets.
 * Each entry point fails, so a call that should never happen is loud.
 */

#include <config.h>

#include <net/FakeSocket.hpp>

#include <cassert>

namespace
{
void neverHere()
{
    assert(!"The fake socket API is for the embedded app; this build uses real sockets.");
}
}

void fakeSocketSetLoggingCallback(void (*)(const std::string&)) { neverHere(); }

int fakeSocketSocket()
{
    neverHere();
    return -1;
}

int fakeSocketPipe2(int[2])
{
    neverHere();
    return -1;
}

void fakeSocketWaitAny(int) { neverHere(); }

int fakeSocketPoll(struct pollfd*, int, int)
{
    neverHere();
    return -1;
}

int fakeSocketListen(int)
{
    neverHere();
    return -1;
}

int fakeSocketConnect(int, int)
{
    neverHere();
    return -1;
}

int fakeSocketAccept4(int)
{
    neverHere();
    return -1;
}

int fakeSocketPeer(int)
{
    neverHere();
    return -1;
}

ssize_t fakeSocketAvailableDataLength(int)
{
    neverHere();
    return -1;
}

ssize_t fakeSocketRead(int, void*, size_t)
{
    neverHere();
    return -1;
}

ssize_t fakeSocketWrite(int, const void*, size_t)
{
    neverHere();
    return -1;
}

int fakeSocketShutdown(int)
{
    neverHere();
    return -1;
}

int fakeSocketClose(int)
{
    neverHere();
    return -1;
}

void fakeSocketDumpState() { neverHere(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
