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

#ifndef INCLUDED_UNOTOOLS_READWRITEMUTEXGUARD_HXX
#define INCLUDED_UNOTOOLS_READWRITEMUTEXGUARD_HXX

#include <osl/mutex.hxx>

namespace utl {

class ReadWriteGuard;
class ReadWriteMutex
{
    friend class ReadWriteGuard;

            sal_uInt32          nReadCount;
            sal_uInt32          nBlockCriticalCount;
            ::osl::Mutex*       pMutex;
            ::osl::Mutex*       pWriteMutex;

public:
                                ReadWriteMutex()
                                    : nReadCount(0)
                                    , nBlockCriticalCount(0)
                                    , pMutex( new ::osl::Mutex )
                                    , pWriteMutex( new ::osl::Mutex )
                                    {}
                                ~ReadWriteMutex()
                                    {
                                        delete pMutex;
                                        delete pWriteMutex;
                                    }
};


namespace ReadWriteGuardMode {
const sal_Int32 nWrite          = 0x01;
const sal_Int32 nCriticalChange = 0x02 | nWrite;
const sal_Int32 nBlockCritical  = 0x04;     // only a block, not a read, exclusive flag!
}

/** Enable multiple threads to read simultaneously, but a write blocks all
    other reads and writes, and a read blocks any write.
    Used in I18N wrappers to be able to maintain a single instance of a wrapper
    for the standard Office locale.
    NEVER construct a writing guard if there is already a reading guard in the
    same context, the following will dead lock EVEN IN THE SAME THREAD!
    void foo()
    {
        ReadWriteGuard aGuard1( aMutex );
        bar();
    }
    void bar()
    {
        // waits forever for aGuard1
        ReadWriteGuard aGuard2( aMutex, ReadWriteGuardMode::nWrite );
    }
 */
class ReadWriteGuard
{
            ReadWriteMutex&     rMutex;
            sal_Int32           nMode;
public:
                                ReadWriteGuard(
                                    ReadWriteMutex& rMutex,
                                    sal_Int32 nRequestMode = 0  // read only
                                    );
                                ~ReadWriteGuard();

    /** Be careful with this, it does wait for ANY read to complete.
        The following will dead lock EVEN IN THE SAME THREAD!
        void foo()
        {
            ReadWriteGuard aGuard1( aMutex );
            bar();
        }
        void bar()
        {
            ReadWriteGuard aGuard2( aMutex );
            aGuard2.changeReadToWrite();    // waits forever for aGuard1
        }
     */
            void                changeReadToWrite();
};

}   // namespace utl

#endif // INCLUDED_UNOTOOLS_READWRITEMUTEXGUARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
