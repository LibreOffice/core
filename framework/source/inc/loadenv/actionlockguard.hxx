/*************************************************************************
 *
 *  $RCSfile: actionlockguard.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-01-28 14:35:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_LOADENV_ACTIONLOCKGUARD_HXX_
#define __FRAMEWORK_LOADENV_ACTIONLOCKGUARD_HXX_

//_______________________________________________
// includes of own project

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

//_______________________________________________
// includes of uno interface

#ifndef _COM_SUN_STAR_DOCUMENT_XACTIONLOCKABLE_HPP_
#include <com/sun/star/document/XActionLockable.hpp>
#endif

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

            @return TRUE, if new resource could be set and locked.
                    FALSE otherwhise.
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

            @return TRUE, if new resource could be set and locked.
                    FALSE otherwhise.
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
