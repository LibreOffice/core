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

#include <AccessibleContextBase.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <toolkit/helper/convert.hxx>
#include <svl/hint.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <utility>
#include <vcl/unohelp.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

/**
 The listener is an internal class to prevent reference-counting cycles and therefore memory leaks.
*/
typedef cppu::WeakComponentImplHelper<
                css::accessibility::XAccessibleEventListener
                > ScAccessibleContextBaseEventListenerWeakImpl;
class ScAccessibleContextBase::ScAccessibleContextBaseEventListener : public cppu::BaseMutex, public ScAccessibleContextBaseEventListenerWeakImpl
{
public:
    ScAccessibleContextBaseEventListener(ScAccessibleContextBase& rBase)
        : ScAccessibleContextBaseEventListenerWeakImpl(m_aMutex), mrBase(rBase) {}

    using WeakComponentImplHelperBase::disposing;

    ///=====  XAccessibleEventListener  ========================================

    virtual void SAL_CALL disposing( const lang::EventObject& rSource ) override
    {
        SolarMutexGuard aGuard;
        if (rSource.Source == mrBase.mxParent)
            dispose();
    }

    virtual void SAL_CALL
        notifyEvent(
        const css::accessibility::AccessibleEventObject& /*aEvent*/ ) override {}
private:
    ScAccessibleContextBase& mrBase;
};


ScAccessibleContextBase::ScAccessibleContextBase(
                                                 uno::Reference<XAccessible> xParent,
                                                 const sal_Int16 aRole)
                                                 :
    ScAccessibleContextBaseWeakImpl(m_aMutex),
    mxParent(std::move(xParent)),
    mnClientId(0),
    maRole(aRole)
{
}

ScAccessibleContextBase::~ScAccessibleContextBase()
{
    if (!IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_atomic_increment( &m_refCount );
        // call dispose to inform object which have a weak reference to this object
        dispose();
    }
}

void ScAccessibleContextBase::Init()
{
    // hold reference to make sure that the destructor is not called
    uno::Reference< XAccessibleContext > xKeepAlive(this);

    if (mxParent.is())
    {
        uno::Reference< XAccessibleEventBroadcaster > xBroadcaster (mxParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xBroadcaster.is())
        {
            if (!mxEventListener)
                mxEventListener = new ScAccessibleContextBaseEventListener(*this);
            xBroadcaster->addAccessibleEventListener(mxEventListener);
        }
    }
    msName = createAccessibleName();
    msDescription = createAccessibleDescription();
}

void SAL_CALL ScAccessibleContextBase::disposing()
{
    SolarMutexGuard aGuard;
//  CommitDefunc(); not necessary and should not be send, because it cost a lot of time

    // hold reference to make sure that the destructor is not called
    uno::Reference< XAccessibleContext > xKeepAlive(this);

    if ( mnClientId )
    {
        sal_Int32 nTemClientId(mnClientId);
        mnClientId =  0;
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( nTemClientId, *this );
    }

    if (mxParent.is())
    {
        uno::Reference< XAccessibleEventBroadcaster > xBroadcaster (mxParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xBroadcaster && mxEventListener)
            xBroadcaster->removeAccessibleEventListener(mxEventListener);
        mxParent = nullptr;
    }
}


//=====  SfxListener  =====================================================

void ScAccessibleContextBase::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if (rHint.GetId() == SfxHintId::Dying)
    {
        // it seems the Broadcaster is dying, since the view is dying
        dispose();
    }
}

//=====  XAccessible  =========================================================

uno::Reference< XAccessibleContext> SAL_CALL
    ScAccessibleContextBase::getAccessibleContext()
{
    return this;
}

//=====  XAccessibleComponent  ================================================

sal_Bool SAL_CALL ScAccessibleContextBase::containsPoint(const awt::Point& rPoint )
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return tools::Rectangle (Point(), GetBoundingBox().GetSize()).Contains(VCLPoint(rPoint));
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleContextBase::getAccessibleAtPoint(
        const awt::Point& /* rPoint */ )
{
    OSL_FAIL("not implemented");
    return uno::Reference<XAccessible>();
}

awt::Rectangle SAL_CALL ScAccessibleContextBase::getBounds(  )
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return AWTRectangle(GetBoundingBox());
}

awt::Point SAL_CALL ScAccessibleContextBase::getLocation(  )
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return AWTPoint(GetBoundingBox().TopLeft());
}

awt::Point SAL_CALL ScAccessibleContextBase::getLocationOnScreen(  )
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return AWTPoint(GetBoundingBoxOnScreen().TopLeft());
}

awt::Size SAL_CALL ScAccessibleContextBase::getSize(  )
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return AWTSize(GetBoundingBox().GetSize());
}

bool ScAccessibleContextBase::isShowing(  )
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    bool bShowing(false);
    if (mxParent.is())
    {
        uno::Reference<XAccessibleComponent> xParentComponent (mxParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xParentComponent.is())
        {
            tools::Rectangle aParentBounds(VCLRectangle(xParentComponent->getBounds()));
            tools::Rectangle aBounds(VCLRectangle(getBounds()));
            bShowing = aBounds.Overlaps(aParentBounds);
        }
    }
    return bShowing;
}

bool ScAccessibleContextBase::isVisible()
{
    return true;
}

void SAL_CALL ScAccessibleContextBase::grabFocus(  )
{
    OSL_FAIL("not implemented");
}

sal_Int32 SAL_CALL ScAccessibleContextBase::getForeground(  )
{
    return sal_Int32(COL_BLACK);
}

sal_Int32 SAL_CALL ScAccessibleContextBase::getBackground(  )
{
    return sal_Int32(COL_WHITE);
}

//=====  XAccessibleContext  ==================================================

sal_Int64 SAL_CALL ScAccessibleContextBase::getAccessibleChildCount()
{
    OSL_FAIL("should be implemented in the abrevated class");
    return 0;
}

uno::Reference<XAccessible> SAL_CALL
    ScAccessibleContextBase::getAccessibleChild(sal_Int64 /* nIndex */)
{
    OSL_FAIL("should be implemented in the abrevated class");
    return uno::Reference<XAccessible>();
}

uno::Reference<XAccessible> SAL_CALL
       ScAccessibleContextBase::getAccessibleParent()
{
    return mxParent;
}

sal_Int64 SAL_CALL
       ScAccessibleContextBase::getAccessibleIndexInParent()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    //  Use a simple but slow solution for now.  Optimize later.
   //   Return -1 to indicate that this object's parent does not know about the
   //   object.
    sal_Int64 nIndex(-1);

    //  Iterate over all the parent's children and search for this object.
    if (mxParent.is())
    {
        uno::Reference<XAccessibleContext> xParentContext (
            mxParent->getAccessibleContext());
        if (xParentContext.is())
        {
            sal_Int64 nChildCount = xParentContext->getAccessibleChildCount();
            for (sal_Int64 i=0; i<nChildCount; ++i)
            {
                uno::Reference<XAccessible> xChild (xParentContext->getAccessibleChild (i));
                if (xChild.is() && xChild.get() == this)
                    nIndex = i;
            }
        }
    }

    return nIndex;
}

sal_Int16 SAL_CALL
    ScAccessibleContextBase::getAccessibleRole()
{
    return maRole;
}

OUString SAL_CALL
       ScAccessibleContextBase::getAccessibleDescription()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (msDescription.isEmpty())
    {
        OUString sDescription(createAccessibleDescription());

        if (msDescription != sDescription)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::DESCRIPTION_CHANGED;
            aEvent.Source = uno::Reference< XAccessibleContext >(this);
            aEvent.OldValue <<= msDescription;
            aEvent.NewValue <<= sDescription;

            msDescription = sDescription;

            CommitChange(aEvent);
        }
    }
    return msDescription;
}

OUString SAL_CALL
       ScAccessibleContextBase::getAccessibleName()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (msName.isEmpty())
    {
        OUString sName(createAccessibleName());
        OSL_ENSURE(!sName.isEmpty(), "We should give always a name.");

        if (msName != sName)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::NAME_CHANGED;
            aEvent.Source = uno::Reference< XAccessibleContext >(this);
            aEvent.OldValue <<= msName;
            aEvent.NewValue <<= sName;

            msName = sName;

            CommitChange(aEvent);
        }
    }
    return msName;
}

uno::Reference<XAccessibleRelationSet> SAL_CALL
       ScAccessibleContextBase::getAccessibleRelationSet()
{
    return new utl::AccessibleRelationSetHelper();
}

sal_Int64 SAL_CALL ScAccessibleContextBase::getAccessibleStateSet()
{
    return 0;
}

lang::Locale SAL_CALL
       ScAccessibleContextBase::getLocale()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (mxParent.is())
    {
        uno::Reference<XAccessibleContext> xParentContext (
            mxParent->getAccessibleContext());
        if (xParentContext.is())
            return xParentContext->getLocale ();
    }

    //  No locale and no parent.  Therefore throw exception to indicate this
    //  cluelessness.
    throw IllegalAccessibleComponentStateException ();
}

    //=====  XAccessibleEventBroadcaster  =====================================

void SAL_CALL
       ScAccessibleContextBase::addAccessibleEventListener(
           const uno::Reference<XAccessibleEventListener>& xListener)
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        IsObjectValid();
        if (!IsDefunc())
        {
            if (!mnClientId)
                mnClientId = comphelper::AccessibleEventNotifier::registerClient( );
            comphelper::AccessibleEventNotifier::addEventListener( mnClientId, xListener );
        }
    }
}

void SAL_CALL
       ScAccessibleContextBase::removeAccessibleEventListener(
        const uno::Reference<XAccessibleEventListener>& xListener)
{
    if (!xListener.is())
        return;

    SolarMutexGuard aGuard;
    if (IsDefunc() || !mnClientId)
        return;

    sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( mnClientId, xListener );
    if ( !nListenerCount )
    {
        // no listeners anymore
        // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
        // and at least to us not firing any events anymore, in case somebody calls
        // NotifyAccessibleEvent, again
        comphelper::AccessibleEventNotifier::revokeClient( mnClientId );
        mnClientId = 0;
    }
}

// XServiceInfo
OUString SAL_CALL ScAccessibleContextBase::getImplementationName()
{
    return u"ScAccessibleContextBase"_ustr;
}

sal_Bool SAL_CALL ScAccessibleContextBase::supportsService(const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}

uno::Sequence< OUString> SAL_CALL
       ScAccessibleContextBase::getSupportedServiceNames()
{
    return {u"com.sun.star.accessibility.Accessible"_ustr,
            u"com.sun.star.accessibility.AccessibleContext"_ustr};
}

//=====  internal  ============================================================

OUString
    ScAccessibleContextBase::createAccessibleDescription()
{
    OSL_FAIL("should be implemented in the abrevated class");
    return OUString();
}

OUString ScAccessibleContextBase::createAccessibleName()
{
    OSL_FAIL("should be implemented in the abrevated class");
    return OUString();
}

void ScAccessibleContextBase::CommitChange(const AccessibleEventObject& rEvent) const
{
    if (mnClientId)
        comphelper::AccessibleEventNotifier::addEvent( mnClientId, rEvent );
}

void ScAccessibleContextBase::CommitFocusGained() const
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::STATE_CHANGED;
    aEvent.Source = uno::Reference< XAccessibleContext >(const_cast<ScAccessibleContextBase*>(this));
    aEvent.NewValue <<= AccessibleStateType::FOCUSED;

    CommitChange(aEvent);
}

void ScAccessibleContextBase::CommitFocusLost() const
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::STATE_CHANGED;
    aEvent.Source = uno::Reference< XAccessibleContext >(const_cast<ScAccessibleContextBase*>(this));
    aEvent.OldValue <<= AccessibleStateType::FOCUSED;

    CommitChange(aEvent);
}

AbsoluteScreenPixelRectangle ScAccessibleContextBase::GetBoundingBoxOnScreen() const
{
    OSL_FAIL("not implemented");
    return AbsoluteScreenPixelRectangle();
}

tools::Rectangle ScAccessibleContextBase::GetBoundingBox() const
{
    OSL_FAIL("not implemented");
    return tools::Rectangle();
}

void ScAccessibleContextBase::IsObjectValid() const
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        throw lang::DisposedException();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
