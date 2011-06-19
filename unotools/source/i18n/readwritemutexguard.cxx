/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"
#include "unotools/readwritemutexguard.hxx"
#include <tools/debug.hxx>


namespace utl {

ReadWriteGuard::ReadWriteGuard( ReadWriteMutex& rMutexP,
            sal_Int32 nRequestMode )
        : rMutex( rMutexP )
{
    // don't do anything until a pending write completed (or another
    // ReadWriteGuard leaves the ctor phase)
    ::osl::MutexGuard aGuard( rMutex.pWriteMutex );
    nMode = nRequestMode;
    if ( nMode & ReadWriteGuardMode::nWrite )
    {
        rMutex.pWriteMutex->acquire();
        // wait for any read to complete
// TODO: set up a waiting thread instead of a loop
        sal_Bool bWait = sal_True;
        do
        {
            rMutex.pMutex->acquire();
            bWait = (rMutex.nReadCount != 0);
            if ( nMode & ReadWriteGuardMode::nCriticalChange )
                bWait |= (rMutex.nBlockCriticalCount != 0);
            rMutex.pMutex->release();
        } while ( bWait );
    }
    else if ( nMode & ReadWriteGuardMode::nBlockCritical )
    {
        rMutex.pMutex->acquire();
        ++rMutex.nBlockCriticalCount;
        rMutex.pMutex->release();
    }
    else
    {
        rMutex.pMutex->acquire();
        ++rMutex.nReadCount;
        rMutex.pMutex->release();
    }
}


ReadWriteGuard::~ReadWriteGuard()
{
    if ( nMode & ReadWriteGuardMode::nWrite )
        rMutex.pWriteMutex->release();
    else if ( nMode & ReadWriteGuardMode::nBlockCritical )
    {
        rMutex.pMutex->acquire();
        --rMutex.nBlockCriticalCount;
        rMutex.pMutex->release();
    }
    else
    {
        rMutex.pMutex->acquire();
        --rMutex.nReadCount;
        rMutex.pMutex->release();
    }
}


void ReadWriteGuard::changeReadToWrite()
{
    sal_Bool bOk = !(nMode & (ReadWriteGuardMode::nWrite | ReadWriteGuardMode::nBlockCritical));
    DBG_ASSERT( bOk, "ReadWriteGuard::changeReadToWrite: can't" );
    if ( bOk )
    {
        // MUST release read before acquiring write mutex or dead lock would
        // occur if there was a write in another thread waiting for this read
        // to complete.
        rMutex.pMutex->acquire();
        --rMutex.nReadCount;
        rMutex.pMutex->release();

        rMutex.pWriteMutex->acquire();
        nMode |= ReadWriteGuardMode::nWrite;
        // wait for any other read to complete
// TODO: set up a waiting thread instead of a loop
        sal_Bool bWait = sal_True;
        do
        {
            rMutex.pMutex->acquire();
            bWait = (rMutex.nReadCount != 0);
            rMutex.pMutex->release();
        } while ( bWait );
    }
}

}   // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
