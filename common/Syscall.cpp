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

#include "Syscall.hpp"

#include <cerrno>

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>


/**
 * Internal helper functions, for the fallbacks that the platform makes necessary.
 */
#if !defined(__linux__) || !HAVE_PIPE2
namespace {


/**
 * Set FD_CLOEXEC and O_NONBLOCK on one file descriptor.
 *
 * Called "unsafe" because it keeps the file descriptor open on error, possibly leading to leaks.
 */
bool unsafe_set_fd_cloexec_nonblock(int fd, bool cloexec, bool nonblock) {
    // Set FD_CLOEXEC if the user wants it
    if (cloexec)
    {
        int fd_flags = fcntl(fd, F_GETFD);
        if (fd_flags == -1)
            return false;

        fd_flags |= FD_CLOEXEC;
        if (fcntl(fd, F_SETFD, fd_flags) == -1)
            return false;
    }

    // Set O_NONBLOCK if the user wants it
    if (nonblock)
    {
        int fl_flags = fcntl(fd, F_GETFL);
        if (fl_flags == -1)
            return false;

        fl_flags |= O_NONBLOCK;
        if (fcntl(fd, F_SETFL, fl_flags) == -1)
            return false;
    }

    return true;
}

/**
 * Set CLOEXEC or NONBLOCK on both sides of the pipe/socket/...
 */
bool set_fds_cloexec_nonblock(int fds[2], bool cloexec, bool nonblock) {
    for (int i = 0; i < 2; i++) {
        bool ret = unsafe_set_fd_cloexec_nonblock(fds[i], cloexec, nonblock);
        if (!ret) {
            int saved_errno = errno;
            close(fds[0]);
            close(fds[1]);
            errno = saved_errno;

            return false;
        }
    }

    return true;
}

}
#endif

#if !defined(__linux__)
namespace {

/**
 * Set FD_CLOEXEC and O_NONBLOCK on one file descriptor.
 */
bool set_fd_cloexec_nonblock(int fd, bool cloexec, bool nonblock) {
    bool ret = unsafe_set_fd_cloexec_nonblock(fd, cloexec, nonblock);
    if (!ret) {
        int saved_errno = errno;
        close(fd);
        errno = saved_errno;
    }

    return ret;
}

}
#endif

int Syscall::accept_cloexec_nonblock(int socket, struct sockaddr *address, socklen_t *address_len)
{
#if defined(__linux__)
    return accept4(socket, address, address_len, SOCK_CLOEXEC | SOCK_NONBLOCK);
#else
    int fd = ::accept(socket, address, address_len);
    if (fd < 0)
        return fd;

    return set_fd_cloexec_nonblock(fd, true, true)? fd: -1;
#endif
}

int Syscall::get_peer_pid(int socket) {
#ifdef __linux__
    struct ucred creds;
    socklen_t credSize = sizeof(struct ucred);
    if (getsockopt(socket, SOL_SOCKET, SO_PEERCRED, &creds, &credSize) < 0)
        return -1;

    return creds.pid;
#elif defined(__FreeBSD__)
    struct xucred creds;
    socklen_t credSize = sizeof(struct xucred);
    if (getsockopt(socket, SOL_LOCAL, LOCAL_PEERCRED, &creds, &credSize) < 0)
        return -1;

    return creds.cr_pid;
#elif defined(__APPLE__)
    int pid = -1;
    socklen_t pidLen = sizeof(pid);

    // Retrieve the PID of the peer connected on this Unix-domain socket
    if (getsockopt(socket, SOL_LOCAL, LOCAL_PEERPID, &pid, &pidLen) < 0)
        return -1;

    return pid;
#elif defined __EMSCRIPTEN__
    (void) socket;

    // Every socket in the browser build is a fake socket inside this one process, so the peer is
    // this process itself and there is nothing to look up.
    return -1;
#else
#error Implement for your platform
#endif
}

/// Implementation of pipe2() for platforms that don't have it (like macOS)
int Syscall::pipe2(int pipefd[2], int flags)
{
#if HAVE_PIPE2
    return ::pipe2(pipefd, flags);
#else
    if (pipe(pipefd) < 0)
        return -1;

    return set_fds_cloexec_nonblock(pipefd, flags & O_CLOEXEC, flags & O_NONBLOCK)? 0: -1;
#endif
}

int Syscall::socket_cloexec_nonblock(int domain, int type, int protocol) {
#ifdef __linux__
    return ::socket(domain, type | SOCK_NONBLOCK | SOCK_CLOEXEC, protocol);
#else
    int fd = ::socket(domain, type, protocol);
    if (fd < 0)
        return fd;

    return set_fd_cloexec_nonblock(fd, true, true)? fd: -1;
#endif
}

int Syscall::socketpair_cloexec_nonblock(int domain, int type, int protocol, int socket_vector[2])
{
#ifdef __linux__
    return ::socketpair(domain, type | SOCK_NONBLOCK | SOCK_CLOEXEC, protocol, socket_vector);
#else
    int rc = ::socketpair(domain, type, protocol, socket_vector);
    if (rc < 0)
        return rc;

    return set_fds_cloexec_nonblock(socket_vector, true, true)? 0: -1;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
