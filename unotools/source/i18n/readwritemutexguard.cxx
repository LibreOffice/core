/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: readwritemutexguard.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:25:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

#ifndef INCLUDED_UNOTOOLS_READWRITEMUTEXGUARD_HXX
#include "unotools/readwritemutexguard.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


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
