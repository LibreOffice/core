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
#include "precompiled_sd.hxx"

#include "UpdateLockManager.hxx"

#include "MutexOwner.hxx"
#include "ViewShellBase.hxx"
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XLayoutManagerEventBroadcaster.hpp>
#include <com/sun/star/frame/LayoutManagerEvents.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/compbase1.hxx>

#include <vcl/timer.hxx>
#include <sfx2/viewfrm.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

namespace {
typedef cppu::WeakComponentImplHelper1<frame::XLayoutManagerListener> InterfaceBase;
}

namespace sd {


/** This implementation class not only implements the Lock() and Unlock()
    methods but as well listens for the right combination of events to call
    Unlock() when all is ready after the PaneManager has switched (some of)
    its view shells.
*/

class UpdateLockManager::Implementation
    : protected MutexOwner,
      public InterfaceBase
{
public:
    Implementation (ViewShellBase& rBase);
    virtual ~Implementation (void);

    void Lock (void);
    void Unlock (void);
    bool IsLocked (void) const;

    /** Unlock regardless of the current lock level.
    */
    void ForceUnlock (void);

private:
    ViewShellBase& mrBase;
    /// A lock level greater than 0 indicates that the ViewShellBase is locked.
    sal_Int32 mnLockDepth;
    /// The emergency timer to unlock the ViewShellBase when all else fails.
    Timer maTimer;
    /// Remember when to unlock after a layout event from frame::XLayoutManager
    bool mbUnlockOnNextLayout;
    /// Remember whether we are listening to the frame::XLayoutManager
    bool mbListenerIsRegistered;
    /// Remember whether the frame::XLayoutManager is locked.
    bool mbLayouterIsLocked;
    /** We hold a weak reference to the layout manager in order to have
        access to it even when the ViewShellBase object is not valid anymore
        and can not be used to obtain the layout manager.
    */
    WeakReference<frame::XLayoutManager> mxLayoutManager;

    //=====  frame::XLayoutEventListener  =====================================

    /** The event of the layouter are observed to find the best moment for
        unlocking.  This is the first layout after the lock level of the
        layouter drops to one (we hold a lock to it ourselves which we
        release when unlocking).
    */
    virtual void SAL_CALL layoutEvent (
        const lang::EventObject& xSource,
        sal_Int16 eLayoutEvent,
        const Any& rInfo)
        throw (uno::RuntimeException);

    //=====  lang::XEventListener  ============================================
    virtual void SAL_CALL
        disposing (const lang::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing (void);

    /** This is only a fallback to make the office usable when for some
        reason the intended way of unlocking it failed.
    */
    DECL_LINK(Timeout, void*);

    /** Convenience method that finds the layout manager associated with the
        frame that shows the ViewShellBase.
    */
    Reference<frame::XLayoutManager> GetLayoutManager (void);

    Implementation (const Implementation&); // Not implemented.
    Implementation& operator= (const Implementation&); // Not implemented.
};




//===== UpdateLockManager =====================================================

UpdateLockManager::UpdateLockManager (ViewShellBase& rBase)
    : mpImpl(new Implementation(rBase))
{
    mpImpl->acquire();
}



UpdateLockManager::~UpdateLockManager (void)
{
    if (mpImpl != NULL)
    {
        mpImpl->ForceUnlock();
        mpImpl->release();
    }
}




void UpdateLockManager::Disable (void)
{
    if (mpImpl != NULL)
    {
        mpImpl->ForceUnlock();
        mpImpl->release();
        mpImpl = NULL;
    }
}




void UpdateLockManager::Lock (void)
{
    if (mpImpl != NULL)
        mpImpl->Lock();
}




void UpdateLockManager::Unlock (void)
{
    if (mpImpl != NULL)
        mpImpl->Unlock();
}




bool UpdateLockManager::IsLocked (void) const
{
    if (mpImpl != NULL)
        return mpImpl->IsLocked();
    else
        return false;
}



//===== UpdateLock::Implementation ============================================

UpdateLockManager::Implementation::Implementation (ViewShellBase& rBase)
    : InterfaceBase(maMutex),
      mrBase(rBase),
      mnLockDepth(0),
      maTimer(),
      mbUnlockOnNextLayout(false),
      mbListenerIsRegistered(false),
      mbLayouterIsLocked(false)
{
}




UpdateLockManager::Implementation::~Implementation (void)
{
    OSL_ASSERT(mnLockDepth==0);
    ForceUnlock();
}




void UpdateLockManager::Implementation::Lock (void)
{
    ++mnLockDepth;
    if (mnLockDepth == 1)
    {
        Reference<frame::XLayoutManager> xLayouter (GetLayoutManager());
        if (xLayouter.is())
        {
            // Register as event listener.
            Reference<frame::XLayoutManagerEventBroadcaster> xBroadcaster (
                xLayouter, UNO_QUERY);
            if (xBroadcaster.is())
            {
                mbListenerIsRegistered = true;
                xBroadcaster->addLayoutManagerEventListener(
                    Reference<frame::XLayoutManagerListener> (
                        static_cast<XWeak*>(this), UNO_QUERY) );
            }

            // Lock the layout manager.
            mbLayouterIsLocked = true;
            xLayouter->lock();
        }

        // As a fallback, when the notification mechanism does not work (or is
        // incorrectly used) we use a timer that will unlock us eventually.
        maTimer.SetTimeout(5000 /*ms*/);
        maTimer.SetTimeoutHdl(LINK(this,UpdateLockManager::Implementation,Timeout));
        maTimer.Start();
    }
}




void UpdateLockManager::Implementation::Unlock (void)
{
    --mnLockDepth;

    if (mnLockDepth == 0)
    {
        // Stop the timer.  We don't need it anymore.
        maTimer.Stop();

        try
        {
            Reference<frame::XLayoutManager> xLayouter (GetLayoutManager());
            if (xLayouter.is())
            {
                // Detach from the layouter.
                if (mbListenerIsRegistered)
                {
                    Reference<frame::XLayoutManagerEventBroadcaster> xBroadcaster (
                        xLayouter, UNO_QUERY);
                    if (xBroadcaster.is())
                    {
                        mbListenerIsRegistered = false;
                        xBroadcaster->removeLayoutManagerEventListener(
                            Reference<frame::XLayoutManagerListener> (
                                static_cast<XWeak*>(this), UNO_QUERY) );
                    }
                }

                // Unlock the layouter.
                if (mbLayouterIsLocked)
                {
                    mbLayouterIsLocked = false;
                    xLayouter->unlock();
                }
            }
        }
        catch (RuntimeException)
        { }

        // Force a rearrangement of the UI elements of the views.
        mrBase.Rearrange();
    }
}




bool UpdateLockManager::Implementation::IsLocked (void) const
{
    return (mnLockDepth > 0);
}




void UpdateLockManager::Implementation::ForceUnlock (void)
{
    while (IsLocked())
        Unlock();
}




void SAL_CALL UpdateLockManager::Implementation::layoutEvent (
    const lang::EventObject&,
    sal_Int16 eLayoutEvent,
    const Any& rInfo)
    throw (uno::RuntimeException)
{
    switch (eLayoutEvent)
    {
        case frame::LayoutManagerEvents::LOCK:
        {
            sal_Int32 nLockCount;
            rInfo >>= nLockCount;
        }
        break;

        case frame::LayoutManagerEvents::UNLOCK:
        {
            sal_Int32 nLockCount = 0;
            rInfo >>= nLockCount;
            if (nLockCount == 1)
            {
                // The lock count dropped to one.  This means that we are
                // the only one that still holds a lock to the layout
                // manager.  We unlock the layout manager now and the
                // ViewShellBase on the next layout of the layout manager.
                mbUnlockOnNextLayout = true;
                Reference<frame::XLayoutManager> xLayouter (GetLayoutManager());
                if (xLayouter.is() && mbLayouterIsLocked)
                {
                    mbLayouterIsLocked = false;
                    xLayouter->unlock();
                }
            }
        }
        break;

        case frame::LayoutManagerEvents::LAYOUT:
            // Unlock when the layout manager is not still locked.
            if (mbUnlockOnNextLayout)
                Unlock();
            break;
    }
}




void SAL_CALL UpdateLockManager::Implementation::disposing (const lang::EventObject& )
    throw (::com::sun::star::uno::RuntimeException)
{
}




void SAL_CALL UpdateLockManager::Implementation::disposing (void)
{
}




IMPL_LINK(UpdateLockManager::Implementation, Timeout, void*, EMPTYARG)
{
    // This method is only called when all else failed.  We unlock
    // regardless of how deep the lock depth.
    while (mnLockDepth > 0)
        Unlock();
    return 1;
}




Reference< ::com::sun::star::frame::XLayoutManager>
    UpdateLockManager::Implementation::GetLayoutManager (void)
{
    Reference<frame::XLayoutManager> xLayoutManager;

    if (mxLayoutManager.get() == NULL)
    {
        if (mrBase.GetViewFrame()!=NULL)
        {
            Reference<beans::XPropertySet> xFrameProperties (
                mrBase.GetViewFrame()->GetFrame().GetFrameInterface(),
                UNO_QUERY);
            if (xFrameProperties.is())
            {
                try
                {
                    Any aValue (xFrameProperties->getPropertyValue(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LayoutManager"))));
                    aValue >>= xLayoutManager;
                }
                catch (const beans::UnknownPropertyException& rException)
                {
                    (void)rException;
                }
            }
            mxLayoutManager = xLayoutManager;
        }
    }
    else
        xLayoutManager = mxLayoutManager;

    return xLayoutManager;
}




} // end of anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
