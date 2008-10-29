/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: datalock.hxx,v $
 * $Revision: 1.5 $
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

#ifndef CONFIGMGR_DATALOCK_HXX_
#define CONFIGMGR_DATALOCK_HXX_

#include "sal/config.h"

#include "osl/interlck.h"
#include "osl/mutex.hxx"

namespace configmgr {

    class UnoApiLock
    {
        static osl::Mutex aCoreLock;
    public:
        static volatile oslInterlockedCount nHeld;
        UnoApiLock()  { acquire(); }
        ~UnoApiLock() { release(); }

        static osl::Mutex &getLock() { return aCoreLock; }
        static void acquire() { aCoreLock.acquire(); nHeld++; }
        static void release() { nHeld--; aCoreLock.release(); }
        static bool isHeld() { return nHeld != 0; }
    };
    class UnoApiLockReleaser
    {
        oslInterlockedCount mnCount;
    public:
        UnoApiLockReleaser();
        ~UnoApiLockReleaser();
    };
    class UnoApiLockClearable : public UnoApiLock
    {
        bool mbSet;
    public:
        UnoApiLockClearable() : mbSet(true)  { acquire(); }
        ~UnoApiLockClearable() { clear(); }
        void clear() { if (mbSet) { mbSet = false; release(); } }
    };
}

#endif // CONFIGMGR_DATALOCK_HXX_
