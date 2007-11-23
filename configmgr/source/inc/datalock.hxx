/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datalock.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:18:01 $
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

#ifndef CONFIGMGR_DATALOCK_HXX_
#define CONFIGMGR_DATALOCK_HXX_

#include <osl/mutex.hxx>
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

using configmgr::UnoApiLock;

#endif // CONFIGMGR_DATALOCK_HXX_
