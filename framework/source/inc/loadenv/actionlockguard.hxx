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

#ifndef __FRAMEWORK_LOADENV_ACTIONLOCKGUARD_HXX_
#define __FRAMEWORK_LOADENV_ACTIONLOCKGUARD_HXX_

//_______________________________________________
// includes of own project

#include <threadhelp/threadhelpbase.hxx>
#include <threadhelp/resetableguard.hxx>

//_______________________________________________
// includes of uno interface
#include <com/sun/star/document/XActionLockable.hpp>

//_______________________________________________
// includes of an other project

//_______________________________________________
// namespace

namespace framework{

#ifndef css
namespace css = ::com::sun::star;
#endif

//_______________________________________________
// definitions

/** @short  implements a guard, which can use the interface
            <type scope="com::sun::star::document">XActionLockable</type>.

    @descr  This guard should be used to be shure, that any lock will be
            released. Otherwhise the locaked document can hinder the office on shutdown!
*/
class ActionLockGuard : private ThreadHelpBase
{
    //-------------------------------------------
    // member

    private:

        /** @short  points to the object, which can be locked from outside. */
        css::uno::Reference< css::document::XActionLockable > m_xActionLock;

        /** @short  knows if a lock exists on the internal lock object
                    forced by this guard instance. */
        sal_Bool m_bActionLocked;

    //-------------------------------------------
    // interface

    public:

        //---------------------------------------
        /** @short  default ctor to initialize a "non working guard".

            @descr  That can be usefull in cases, where no resource still exists,
                    but will be available next time. Then this guard can be used
                    in a mode "use guard for more then one resources".
         */
        ActionLockGuard()
            : ThreadHelpBase (         )
            , m_bActionLocked(sal_False)
        {
        }

        //---------------------------------------
        /** @short  initialize new guard instance and lock the given resource immediatly.

            @param  xLock
                    points to the outside resource, which should be locked.
         */
        ActionLockGuard(const css::uno::Reference< css::document::XActionLockable >& xLock)
            : ThreadHelpBase (         )
            , m_bActionLocked(sal_False)
        {
            setResource(xLock);
        }

        //---------------------------------------
        /** @short  release this guard instance and make shure, that no lock
                    will exist afterwards on the internal wrapped resource.
         */
        virtual ~ActionLockGuard()
        {
            unlock();
        }

        //---------------------------------------
        /** @short  set a new resource for locking at this guard.

            @descr  This call will fail, if an internal resource already exists
                    and is currently locked.

            @param  xLock
                    points to the outside resource, which should be locked.

            @return sal_True, if new resource could be set and locked.
                    sal_False otherwhise.
         */
        virtual sal_Bool setResource(const css::uno::Reference< css::document::XActionLockable >& xLock)
        {
            // SAFE -> ..........................
            ResetableGuard aMutexLock(m_aLock);

            if (m_bActionLocked || !xLock.is())
                return sal_False;

            m_xActionLock = xLock;
            m_xActionLock->addActionLock();
            m_bActionLocked = m_xActionLock->isActionLocked();
            // <- SAFE ..........................

            return sal_True;
        }

        //---------------------------------------
        /** @short  set a new resource for locking at this guard.

            @descr  This call will fail, if an internal resource already exists
                    and is currently locked.

            @param  xLock
                    points to the outside resource, which should be locked.

            @return sal_True, if new resource could be set and locked.
                    sal_False otherwhise.
         */
        virtual void freeResource()
        {
            // SAFE -> ..........................
            ResetableGuard aMutexLock(m_aLock);

            css::uno::Reference< css::document::XActionLockable > xLock   = m_xActionLock  ;
            sal_Bool                                              bLocked = m_bActionLocked;

            m_xActionLock.clear();
            m_bActionLocked = sal_False;

            aMutexLock.unlock();
            // <- SAFE ..........................

            if (bLocked && xLock.is())
                xLock->removeActionLock();
        }

        //---------------------------------------
        /** @short  lock the internal wrapped resource, if its not already done. */
        virtual void lock()
        {
            // SAFE -> ..........................
            ResetableGuard aMutexLock(m_aLock);

            if (!m_bActionLocked && m_xActionLock.is())
            {
                m_xActionLock->addActionLock();
                m_bActionLocked = m_xActionLock->isActionLocked();
            }
            // <- SAFE ..........................
        }

        //---------------------------------------
        /** @short  unlock the internal wrapped resource, if its not already done. */
        virtual void unlock()
        {
            // SAFE -> ..........................
            ResetableGuard aMutexLock(m_aLock);

            if (m_bActionLocked && m_xActionLock.is())
            {
                m_xActionLock->removeActionLock();
                // dont check for any locks here ...
                // May another guard use the same lock object :-(
                m_bActionLocked = sal_False;
            }
            // <- SAFE ..........................
        }
};

} // namespace framework

#endif // __FRAMEWORK_LOADENV_ACTIONLOCKGUARD_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
