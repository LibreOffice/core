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

#include <unotools/readwritemutexguard.hxx>
#include <tools/debug.hxx>

namespace utl {

ReadWriteGuard::ReadWriteGuard( ReadWriteMutex& rMutexP,
            ReadWriteGuardMode nRequestMode )
        : rMutex( rMutexP )
{
    // don't do anything until a pending write completed (or another
    // ReadWriteGuard leaves the ctor phase)
    ::osl::MutexGuard aGuard( rMutex.maWriteMutex );
    nMode = nRequestMode;
    if ( nMode & ReadWriteGuardMode::Write )
    {
        rMutex.maWriteMutex.acquire();
        // wait for any read to complete
// TODO: set up a waiting thread instead of a loop
        bool bWait = true;
        do
        {
            rMutex.maMutex.acquire();
            bWait = (rMutex.nReadCount != 0);
            if ( nMode & ReadWriteGuardMode::CriticalChange )
                bWait |= (rMutex.nBlockCriticalCount != 0);
            rMutex.maMutex.release();
        } while ( bWait );
    }
    else if ( nMode & ReadWriteGuardMode::BlockCritical )
    {
        rMutex.maMutex.acquire();
        ++rMutex.nBlockCriticalCount;
        rMutex.maMutex.release();
    }
    else
    {
        rMutex.maMutex.acquire();
        ++rMutex.nReadCount;
        rMutex.maMutex.release();
    }
}

ReadWriteGuard::~ReadWriteGuard()
{
    if ( nMode & ReadWriteGuardMode::Write )
        rMutex.maWriteMutex.release();
    else if ( nMode & ReadWriteGuardMode::BlockCritical )
    {
        rMutex.maMutex.acquire();
        --rMutex.nBlockCriticalCount;
        rMutex.maMutex.release();
    }
    else
    {
        rMutex.maMutex.acquire();
        --rMutex.nReadCount;
        rMutex.maMutex.release();
    }
}

void ReadWriteGuard::changeReadToWrite()
{
    bool bOk = !(nMode & (ReadWriteGuardMode::Write | ReadWriteGuardMode::BlockCritical));
    DBG_ASSERT( bOk, "ReadWriteGuard::changeReadToWrite: can't" );
    if ( !bOk )
        return;

    // MUST release read before acquiring write mutex or dead lock would
    // occur if there was a write in another thread waiting for this read
    // to complete.
    rMutex.maMutex.acquire();
    --rMutex.nReadCount;
    rMutex.maMutex.release();

    rMutex.maWriteMutex.acquire();
    nMode |= ReadWriteGuardMode::Write;
    // wait for any other read to complete
// TODO: set up a waiting thread instead of a loop
    bool bWait = true;
    do
    {
        rMutex.maMutex.acquire();
        bWait = (rMutex.nReadCount != 0);
        rMutex.maMutex.release();
    } while ( bWait );
}

}   // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
