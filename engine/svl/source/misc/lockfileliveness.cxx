/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <svl/lockfileliveness.hxx>
#include <osl/file.h>
#include <rtl/string.hxx>

#if !defined(_WIN32)
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace {

#if defined(_WIN32)

// No advisory-lock liveness on Windows: it already holds operating-system share locks.
int openForLock(const OUString&) { return -1; }
bool tryLock(int) { return false; }
void closeFd(int) {}

#else

// Convert the file URL to a system path and open it read-write. A write lock
// (F_WRLCK or flock LOCK_EX) requires the file descriptor to be open for
// writing, so O_RDWR is required here. When the file is not writable open
// fails and the function returns -1.
//
// O_CLOEXEC closes the descriptor across exec. A child process that inherited
// the descriptor would share the same open file description and keep the
// advisory lock alive after this process releases it, so the lock must not
// outlive this process through a child.
int openForLock(const OUString& rLockFileURL)
{
    OUString aSystemPath;
    if (osl_getSystemPathFromFileURL(rLockFileURL.pData, &aSystemPath.pData)
        != osl_File_E_None)
        return -1;
    OString aPath = OUStringToOString(aSystemPath, RTL_TEXTENCODING_UTF8);
    return open(aPath.getStr(), O_RDWR | O_CLOEXEC);
}

// Try to take a non-blocking exclusive lock on nFd. Returns true on success.
bool tryLock(int nFd)
{
#ifdef F_OFD_SETLK
    struct flock aLock{};
    aLock.l_type = F_WRLCK;
    aLock.l_whence = SEEK_SET;
    aLock.l_start = 0;
    aLock.l_len = 0;
    return fcntl(nFd, F_OFD_SETLK, &aLock) == 0;
#else
    return flock(nFd, LOCK_EX | LOCK_NB) == 0;
#endif
}

void closeFd(int nFd) { close(nFd); }

#endif

}

namespace svt {

StaleLockDecision decideStaleLock(std::u16string_view rStoredHost,
                                  std::u16string_view rOwnHost,
                                  LockLivenessProbe eProbe)
{
    if (eProbe == LockLivenessProbe::Unsupported)
        return StaleLockDecision::UseIdentityHeuristic;

    if (rStoredHost != rOwnHost)
        return StaleLockDecision::UseIdentityHeuristic;

    if (eProbe == LockLivenessProbe::Free)
        return StaleLockDecision::TakeOverSilently;

    return StaleLockDecision::LiveKeepLock;
}

LockFileLivenessHandle::~LockFileLivenessHandle() { reset(); }

LockFileLivenessHandle::LockFileLivenessHandle(LockFileLivenessHandle&& rOther) noexcept
    : m_nFd(rOther.m_nFd)
{
    rOther.m_nFd = -1;
}

LockFileLivenessHandle& LockFileLivenessHandle::operator=(LockFileLivenessHandle&& rOther) noexcept
{
    if (this != &rOther)
    {
        reset();
        m_nFd = rOther.m_nFd;
        rOther.m_nFd = -1;
    }
    return *this;
}

void LockFileLivenessHandle::reset()
{
    if (m_nFd >= 0)
    {
        closeFd(m_nFd);
        m_nFd = -1;
    }
}

LockFileLivenessHandle acquireLockFileLiveness(const OUString& rLockFileURL)
{
    int nFd = openForLock(rLockFileURL);
    if (nFd < 0)
        return LockFileLivenessHandle();

    if (!tryLock(nFd))
    {
        closeFd(nFd);
        return LockFileLivenessHandle();
    }

    return LockFileLivenessHandle(nFd);
}

LockLivenessProbe probeLockFileLiveness(const OUString& rLockFileURL)
{
    int nFd = openForLock(rLockFileURL);
    if (nFd < 0)
        return LockLivenessProbe::Unsupported;

    bool bGotLock = tryLock(nFd);
    closeFd(nFd);
    return bGotLock ? LockLivenessProbe::Free : LockLivenessProbe::Held;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
