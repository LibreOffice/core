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

#include <sal/config.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <svl/lockfileliveness.hxx>
#include <osl/file.hxx>
#include <rtl/ustring.hxx>

#if !defined(_WIN32)
#include <sys/wait.h>
#include <unistd.h>
#endif

using namespace svt;

namespace {

// Create an empty file in the system temp area and return its file URL.
OUString makeTempLockFileURL()
{
    OUString aTempURL;
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None,
        osl::FileBase::createTempFile(nullptr, nullptr, &aTempURL));
    return aTempURL;
}

class LockFileLivenessTest : public CppUnit::TestFixture
{
public:
    void deadLocalOwnerIsTakenOverSilently();
    void liveLocalOwnerKeepsLock();
    void foreignHostFallsBackToIdentity();
    void unsupportedProbeFallsBackToIdentity();
    void heldLockReadsAsHeld();
    void releasedLockReadsAsFree();
    void missingTargetReadsAsUnsupported();
    void deadHolderReleasesLock();

    CPPUNIT_TEST_SUITE(LockFileLivenessTest);
    CPPUNIT_TEST(deadLocalOwnerIsTakenOverSilently);
    CPPUNIT_TEST(liveLocalOwnerKeepsLock);
    CPPUNIT_TEST(foreignHostFallsBackToIdentity);
    CPPUNIT_TEST(unsupportedProbeFallsBackToIdentity);
    CPPUNIT_TEST(heldLockReadsAsHeld);
    CPPUNIT_TEST(releasedLockReadsAsFree);
    CPPUNIT_TEST(missingTargetReadsAsUnsupported);
    CPPUNIT_TEST(deadHolderReleasesLock);
    CPPUNIT_TEST_SUITE_END();
};

// A free lock owned by this host means the previous owner died: take over with no prompt.
void LockFileLivenessTest::deadLocalOwnerIsTakenOverSilently()
{
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(StaleLockDecision::TakeOverSilently),
        static_cast<int>(decideStaleLock(u"host-a", u"host-a", LockLivenessProbe::Free)));
}

// A held lock on this host is a genuine second session: do not steal it.
void LockFileLivenessTest::liveLocalOwnerKeepsLock()
{
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(StaleLockDecision::LiveKeepLock),
        static_cast<int>(decideStaleLock(u"host-a", u"host-a", LockLivenessProbe::Held)));
}

// A lock claimed by another host cannot be judged by a local probe.
void LockFileLivenessTest::foreignHostFallsBackToIdentity()
{
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(StaleLockDecision::UseIdentityHeuristic),
        static_cast<int>(decideStaleLock(u"host-b", u"host-a", LockLivenessProbe::Free)));
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(StaleLockDecision::UseIdentityHeuristic),
        static_cast<int>(decideStaleLock(u"host-b", u"host-a", LockLivenessProbe::Held)));
}

// When the platform or filesystem cannot lock, keep the old behaviour.
void LockFileLivenessTest::unsupportedProbeFallsBackToIdentity()
{
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(StaleLockDecision::UseIdentityHeuristic),
        static_cast<int>(decideStaleLock(u"host-a", u"host-a", LockLivenessProbe::Unsupported)));
}

// While a handle holds the lock, a probe must see it as Held.
void LockFileLivenessTest::heldLockReadsAsHeld()
{
    OUString aURL = makeTempLockFileURL();
    LockFileLivenessHandle aHandle = acquireLockFileLiveness(aURL);
    CPPUNIT_ASSERT(aHandle.is());
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(LockLivenessProbe::Held),
        static_cast<int>(probeLockFileLiveness(aURL)));
    osl::File::remove(aURL);
}

// After the holder releases, a probe must see the lock as Free.
void LockFileLivenessTest::releasedLockReadsAsFree()
{
    OUString aURL = makeTempLockFileURL();
    {
        LockFileLivenessHandle aHandle = acquireLockFileLiveness(aURL);
        CPPUNIT_ASSERT(aHandle.is());
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(LockLivenessProbe::Free),
        static_cast<int>(probeLockFileLiveness(aURL)));
    osl::File::remove(aURL);
}

// A target that does not exist cannot be probed.
void LockFileLivenessTest::missingTargetReadsAsUnsupported()
{
    OUString aURL = makeTempLockFileURL();
    osl::File::remove(aURL);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(LockLivenessProbe::Unsupported),
        static_cast<int>(probeLockFileLiveness(aURL)));
}

// A child that holds the lock and calls _exit leaves the lock free, because
// the kernel closes every file descriptor when a process exits. This is the
// same release that frees the lock after a crash.
void LockFileLivenessTest::deadHolderReleasesLock()
{
#if !defined(_WIN32)
    OUString aURL = makeTempLockFileURL();

    pid_t nPid = fork();
    CPPUNIT_ASSERT(nPid >= 0);
    if (nPid == 0)
    {
        // Child: take the lock and die hard, running no destructors.
        LockFileLivenessHandle aHandle = acquireLockFileLiveness(aURL);
        _exit(aHandle.is() ? 0 : 1);
    }

    int nStatus = 0;
    CPPUNIT_ASSERT_EQUAL(nPid, waitpid(nPid, &nStatus, 0));
    bool bExited = WIFEXITED(nStatus);
    CPPUNIT_ASSERT(bExited);
    CPPUNIT_ASSERT_EQUAL(0, WEXITSTATUS(nStatus));

    // The dead child held the lock; it must now be free.
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(LockLivenessProbe::Free),
        static_cast<int>(probeLockFileLiveness(aURL)));
    osl::File::remove(aURL);
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(LockFileLivenessTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
