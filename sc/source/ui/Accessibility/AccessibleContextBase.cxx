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

#include "AccessibleContextBase.hxx"
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <svl/smplhint.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/unohelp.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

ScAccessibleContextBase::ScAccessibleContextBase(
                                                 const uno::Reference<XAccessible>& rxParent,
                                                 const sal_Int16 aRole)
                                                 :
    ScAccessibleContextBaseWeakImpl(m_aMutex),
    mxParent(rxParent),
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
    uno::Reference< XAccessibleContext > xOwnContext(this);

    if (mxParent.is())
    {
        uno::Reference< XAccessibleEventBroadcaster > xBroadcaster (mxParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->addAccessibleEventListener(this);
    }
    msName = createAccessibleName();
    msDescription = createAccessibleDescription();
}

void SAL_CALL ScAccessibleContextBase::disposing()
{
    SolarMutexGuard aGuard;
//  CommitDefunc(); not necessary and should not be send, because it cost a lot of time

    // hold reference to make sure that the destructor is not called
    uno::Reference< XAccessibleContext > xOwnContext(this);

    if ( mnClientId )
    {
        sal_Int32 nTemClientId(mnClientId);
        mnClientId =  0;
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( nTemClientId, *this );
    }

    if (mxParent.is())
    {
        uno::Reference< XAccessibleEventBroadcaster > xBroadcaster (mxParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removeAccessibleEventListener(this);
        mxParent = nullptr;
    }

    ScAccessibleContextBaseWeakImpl::disposing();
}

//=====  XInterface  =====================================================

uno::Any SAL_CALL ScAccessibleContextBase::queryInterface( uno::Type const & rType )
    throw (uno::RuntimeException, std::exception)
{
    uno::Any aAny (ScAccessibleContextBaseWeakImpl::queryInterface(rType));
    return aAny.hasValue() ? aAny : ScAccessibleContextBaseImplEvent::queryInterface(rType);
}

void SAL_CALL ScAccessibleContextBase::acquire()
    throw ()
{
    ScAccessibleContextBaseWeakImpl::acquire();
}

void SAL_CALL ScAccessibleContextBase::release()
    throw ()
{
    ScAccessibleContextBaseWeakImpl::release();
}

//=====  SfxListener  =====================================================

void ScAccessibleContextBase::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if (pSimpleHint)
    {
        if (pSimpleHint->GetId() == SFX_HINT_DYING)
        {
            // it seems the Broadcaster is dying, since the view is dying
            dispose();
        }
    }
}

//=====  XAccessible  =========================================================

uno::Reference< XAccessibleContext> SAL_CALL
    ScAccessibleContextBase::getAccessibleContext()
    throw (uno::RuntimeException, std::exception)
{
    return this;
}

//=====  XAccessibleComponent  ================================================

sal_Bool SAL_CALL ScAccessibleContextBase::containsPoint(const awt::Point& rPoint )
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return Rectangle (Point(), GetBoundingBox().GetSize()).IsInside(VCLPoint(rPoint));
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleContextBase::getAccessibleAtPoint(
        const awt::Point& /* rPoint */ )
        throw (uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
    return uno::Reference<XAccessible>();
}

awt::Rectangle SAL_CALL ScAccessibleContextBase::getBounds(  )
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return AWTRectangle(GetBoundingBox());
}

awt::Point SAL_CALL ScAccessibleContextBase::getLocation(  )
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return AWTPoint(GetBoundingBox().TopLeft());
}

awt::Point SAL_CALL ScAccessibleContextBase::getLocationOnScreen(  )
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return AWTPoint(GetBoundingBoxOnScreen().TopLeft());
}

awt::Size SAL_CALL ScAccessibleContextBase::getSize(  )
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return AWTSize(GetBoundingBox().GetSize());
}

bool SAL_CALL ScAccessibleContextBase::isShowing(  )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    bool bShowing(false);
    if (mxParent.is())
    {
        uno::Reference<XAccessibleComponent> xParentComponent (mxParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xParentComponent.is())
        {
            Rectangle aParentBounds(VCLRectangle(xParentComponent->getBounds()));
            Rectangle aBounds(VCLRectangle(getBounds()));
            bShowing = aBounds.IsOver(aParentBounds);
        }
    }
    return bShowing;
}

bool SAL_CALL ScAccessibleContextBase::isVisible()
    throw (uno::RuntimeException, std::exception)
{
    return true;
}

void SAL_CALL ScAccessibleContextBase::grabFocus(  )
        throw (uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

sal_Int32 SAL_CALL ScAccessibleContextBase::getForeground(  )
        throw (uno::RuntimeException, std::exception)
{
    return COL_BLACK;
}

sal_Int32 SAL_CALL ScAccessibleContextBase::getBackground(  )
        throw (uno::RuntimeException, std::exception)
{
    return COL_WHITE;
}

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL ScAccessibleContextBase::getAccessibleChildCount()
    throw (uno::RuntimeException, std::exception)
{
    OSL_FAIL("should be implemented in the abrevated class");
    return 0;
}

uno::Reference<XAccessible> SAL_CALL
    ScAccessibleContextBase::getAccessibleChild(sal_Int32 /* nIndex */)
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException,
               std::exception)
{
    OSL_FAIL("should be implemented in the abrevated class");
    return uno::Reference<XAccessible>();
}

uno::Reference<XAccessible> SAL_CALL
       ScAccessibleContextBase::getAccessibleParent()
    throw (uno::RuntimeException, std::exception)
{
    return mxParent;
}

sal_Int32 SAL_CALL
       ScAccessibleContextBase::getAccessibleIndexInParent()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    //  Use a simple but slow solution for now.  Optimize later.
   //   Return -1 to indicate that this object's parent does not know about the
   //   object.
    sal_Int32 nIndex(-1);

    //  Iterate over all the parent's children and search for this object.
    if (mxParent.is())
    {
        uno::Reference<XAccessibleContext> xParentContext (
            mxParent->getAccessibleContext());
        if (xParentContext.is())
        {
            sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
            for (sal_Int32 i=0; i<nChildCount; ++i)
            {
                uno::Reference<XAccessible> xChild (xParentContext->getAccessibleChild (i));
                if (xChild.is())
                {
                    if (xChild.get() == this)
                        nIndex = i;
                }
            }
        }
   }

   return nIndex;
}

sal_Int16 SAL_CALL
    ScAccessibleContextBase::getAccessibleRole()
    throw (uno::RuntimeException, std::exception)
{
    return maRole;
}

OUString SAL_CALL
       ScAccessibleContextBase::getAccessibleDescription()
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
{
    return new utl::AccessibleRelationSetHelper();
}

uno::Reference<XAccessibleStateSet> SAL_CALL
        ScAccessibleContextBase::getAccessibleStateSet()
    throw (uno::RuntimeException, std::exception)
{
    return uno::Reference<XAccessibleStateSet>();
}

lang::Locale SAL_CALL
       ScAccessibleContextBase::getLocale()
    throw (IllegalAccessibleComponentStateException,
        uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (!IsDefunc() && mnClientId)
        {
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
    }
}

    //=====  XAccessibleEventListener  ========================================

void SAL_CALL ScAccessibleContextBase::disposing(
    const lang::EventObject& rSource )
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (rSource.Source == mxParent)
        dispose();
}

void SAL_CALL ScAccessibleContextBase::notifyEvent(
        const AccessibleEventObject& /* aEvent */ )
        throw (uno::RuntimeException, std::exception)
{
}

// XServiceInfo
OUString SAL_CALL ScAccessibleContextBase::getImplementationName()
    throw (uno::RuntimeException, std::exception)
{
    return OUString("ScAccessibleContextBase");
}

sal_Bool SAL_CALL ScAccessibleContextBase::supportsService(const OUString& sServiceName)
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, sServiceName);
}

uno::Sequence< OUString> SAL_CALL
       ScAccessibleContextBase::getSupportedServiceNames()
    throw (uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aServiceNames(2);
    OUString* pServiceNames = aServiceNames.getArray();
    if (pServiceNames)
    {
        pServiceNames[0] = "com.sun.star.accessibility.Accessible";
        pServiceNames[1] = "com.sun.star.accessibility.AccessibleContext";
    }

    return aServiceNames;
}

//=====  XTypeProvider  =======================================================

uno::Sequence< uno::Type > SAL_CALL ScAccessibleContextBase::getTypes()
        throw (uno::RuntimeException, std::exception)
{
    return comphelper::concatSequences(ScAccessibleContextBaseWeakImpl::getTypes(), ScAccessibleContextBaseImplEvent::getTypes());
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleContextBase::getImplementationId()
    throw (uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

//=====  internal  ============================================================

OUString SAL_CALL
    ScAccessibleContextBase::createAccessibleDescription()
    throw (uno::RuntimeException, std::exception)
{
    OSL_FAIL("should be implemented in the abrevated class");
    return OUString();
}

OUString SAL_CALL ScAccessibleContextBase::createAccessibleName()
    throw (uno::RuntimeException, std::exception)
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

    vcl::unohelper::NotifyAccessibleStateEventGlobally(aEvent);
}

void ScAccessibleContextBase::CommitFocusLost() const
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::STATE_CHANGED;
    aEvent.Source = uno::Reference< XAccessibleContext >(const_cast<ScAccessibleContextBase*>(this));
    aEvent.OldValue <<= AccessibleStateType::FOCUSED;

    CommitChange(aEvent);

    vcl::unohelper::NotifyAccessibleStateEventGlobally(aEvent);
}

Rectangle ScAccessibleContextBase::GetBoundingBoxOnScreen() const
        throw (uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
    return Rectangle();
}

Rectangle ScAccessibleContextBase::GetBoundingBox() const
        throw (uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
    return Rectangle();
}

void ScAccessibleContextBase::IsObjectValid() const
        throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        throw lang::DisposedException();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
