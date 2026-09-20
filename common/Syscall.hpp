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

#include <sys/socket.h>

/**
 * This is a place for platform-dependent syscalls, to be able to extend them conveniently.
 */
namespace Syscall {

    /**
     * Implement an equivalent of accept4() with CLOEXEC and NONBLOCK set.
     *
     * @return the accepted socket, or -1 on error
     */
    int accept_cloexec_nonblock(int socket, struct sockaddr *address, socklen_t *address_len);

    /**
     * Retrieve the PID of the peer connected on this Unix-domain socket.
     *
     * @return peer's PID, or -1 on error
     */
    int get_peer_pid(int socket);

    /**
     * Implement pipe2() on platforms that don't have it.
     *
     * @return 0 on success, -1 on error
     */
    int pipe2(int pipefd[2], int flags);

    /**
     * Implement socket() with CLOEXEC and NONBLOCK on platforms that don't have those flags.
     *
     * @return file descriptor of the socket, or -1 on error
     */
    int socket_cloexec_nonblock(int domain, int type, int protocol);

    /**
     * Implement socket_pair() with CLOEXEC and NONBLOCK on platforms that don't have those flags.
     *
     * @return 0 on success, -1 on error
     */
    int socketpair_cloexec_nonblock(int domain, int type, int protocol, int socket_vector[2]);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
