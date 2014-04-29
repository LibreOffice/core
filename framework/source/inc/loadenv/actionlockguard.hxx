/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

            @descr  That can be useful in cases, where no resource still exists,
                    but will be available next time. Then this guard can be used
                    in a mode "use guard for more then one resources".
         */
        ActionLockGuard()
            : ThreadHelpBase (         )
            , m_bActionLocked(sal_False)
        {
        }

        //---------------------------------------
        /** @short  initialize new guard instance and lock the given resource immediately.

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
