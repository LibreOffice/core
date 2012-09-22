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
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/unohelp.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <vcl/svapp.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

DBG_NAME(ScAccessibleContextBase)

ScAccessibleContextBase::ScAccessibleContextBase(
                                                 const uno::Reference<XAccessible>& rxParent,
                                                 const sal_Int16 aRole)
                                                 :
    ScAccessibleContextBaseWeakImpl(m_aMutex),
    mxParent(rxParent),
    mnClientId(0),
    maRole(aRole)
{
    DBG_CTOR(ScAccessibleContextBase, NULL);
}


ScAccessibleContextBase::~ScAccessibleContextBase(void)
{
    DBG_DTOR(ScAccessibleContextBase, NULL);

    if (!IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_atomic_increment( &m_refCount );
        // call dispose to inform object wich have a weak reference to this object
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
            xBroadcaster->addEventListener(this);
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
            xBroadcaster->removeEventListener(this);
        mxParent = NULL;
    }

    ScAccessibleContextBaseWeakImpl::disposing();
}

//=====  XInterface  =====================================================

uno::Any SAL_CALL ScAccessibleContextBase::queryInterface( uno::Type const & rType )
    throw (uno::RuntimeException)
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
    if (rHint.ISA( SfxSimpleHint ) )
    {
        const SfxSimpleHint& rRef = (const SfxSimpleHint&)rHint;
        if (rRef.GetId() == SFX_HINT_DYING)
        {
            // it seems the Broadcaster is dying, since the view is dying
            dispose();
        }
    }
}

//=====  XAccessible  =========================================================

uno::Reference< XAccessibleContext> SAL_CALL
    ScAccessibleContextBase::getAccessibleContext(void)
    throw (uno::RuntimeException)
{
    return this;
}

//=====  XAccessibleComponent  ================================================

sal_Bool SAL_CALL ScAccessibleContextBase::containsPoint(const awt::Point& rPoint )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return Rectangle (Point(), GetBoundingBox().GetSize()).IsInside(VCLPoint(rPoint));
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleContextBase::getAccessibleAtPoint(
        const awt::Point& /* rPoint */ )
        throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented");
    return uno::Reference<XAccessible>();
}

awt::Rectangle SAL_CALL ScAccessibleContextBase::getBounds(  )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return AWTRectangle(GetBoundingBox());
}

awt::Point SAL_CALL ScAccessibleContextBase::getLocation(  )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return AWTPoint(GetBoundingBox().TopLeft());
}

awt::Point SAL_CALL ScAccessibleContextBase::getLocationOnScreen(  )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return AWTPoint(GetBoundingBoxOnScreen().TopLeft());
}

awt::Size SAL_CALL ScAccessibleContextBase::getSize(  )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return AWTSize(GetBoundingBox().GetSize());
}

sal_Bool SAL_CALL ScAccessibleContextBase::isShowing(  )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    sal_Bool bShowing(false);
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

sal_Bool SAL_CALL ScAccessibleContextBase::isVisible(  )
        throw (uno::RuntimeException)
{
    return sal_True;
}

void SAL_CALL ScAccessibleContextBase::grabFocus(  )
        throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

sal_Int32 SAL_CALL ScAccessibleContextBase::getForeground(  )
        throw (uno::RuntimeException)
{
    return COL_BLACK;
}

sal_Int32 SAL_CALL ScAccessibleContextBase::getBackground(  )
        throw (uno::RuntimeException)
{
    return COL_WHITE;
}

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL
       ScAccessibleContextBase::getAccessibleChildCount(void)
    throw (uno::RuntimeException)
{
    OSL_FAIL("should be implemented in the abrevated class");
    return 0;
}

uno::Reference<XAccessible> SAL_CALL
    ScAccessibleContextBase::getAccessibleChild(sal_Int32 /* nIndex */)
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    OSL_FAIL("should be implemented in the abrevated class");
    return uno::Reference<XAccessible>();
}

uno::Reference<XAccessible> SAL_CALL
       ScAccessibleContextBase::getAccessibleParent(void)
    throw (uno::RuntimeException)
{
    return mxParent;
}

sal_Int32 SAL_CALL
       ScAccessibleContextBase::getAccessibleIndexInParent(void)
    throw (uno::RuntimeException)
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
    ScAccessibleContextBase::getAccessibleRole(void)
    throw (uno::RuntimeException)
{
    return maRole;
}

::rtl::OUString SAL_CALL
       ScAccessibleContextBase::getAccessibleDescription(void)
    throw (uno::RuntimeException)
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
       ScAccessibleContextBase::getAccessibleName(void)
    throw (uno::RuntimeException)
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
       ScAccessibleContextBase::getAccessibleRelationSet(void)
    throw (uno::RuntimeException)
{
    return new utl::AccessibleRelationSetHelper();
}

uno::Reference<XAccessibleStateSet> SAL_CALL
        ScAccessibleContextBase::getAccessibleStateSet(void)
    throw (uno::RuntimeException)
{
    return uno::Reference<XAccessibleStateSet>();
}

lang::Locale SAL_CALL
       ScAccessibleContextBase::getLocale(void)
    throw (IllegalAccessibleComponentStateException,
        uno::RuntimeException)
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
       ScAccessibleContextBase::addEventListener(
           const uno::Reference<XAccessibleEventListener>& xListener)
    throw (uno::RuntimeException)
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
       ScAccessibleContextBase::removeEventListener(
        const uno::Reference<XAccessibleEventListener>& xListener)
    throw (uno::RuntimeException)
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
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (rSource.Source == mxParent)
        dispose();
}

void SAL_CALL ScAccessibleContextBase::notifyEvent(
        const AccessibleEventObject& /* aEvent */ )
        throw (uno::RuntimeException)
{
}

//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
       ScAccessibleContextBase::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleContextBase"));
}

sal_Bool SAL_CALL
     ScAccessibleContextBase::supportsService(const OUString& sServiceName)
    throw (uno::RuntimeException)
{
    //  Iterate over all supported service names and return true if on of them
    //  matches the given name.
    uno::Sequence< ::rtl::OUString> aSupportedServices (
        getSupportedServiceNames ());
    sal_Int32 nLength(aSupportedServices.getLength());
    const OUString* pServiceNames = aSupportedServices.getConstArray();
    for (int i=0; i<nLength; ++i, ++pServiceNames)
        if (sServiceName == *pServiceNames)
            return sal_True;
    return false;
}

uno::Sequence< ::rtl::OUString> SAL_CALL
       ScAccessibleContextBase::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    uno::Sequence<OUString> aServiceNames(2);
    OUString* pServiceNames = aServiceNames.getArray();
    if (pServiceNames)
    {
        pServiceNames[0] = OUString(RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.accessibility.Accessible"));
        pServiceNames[1] = OUString(RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.accessibility.AccessibleContext"));
    }

    return aServiceNames;
}

//=====  XTypeProvider  =======================================================

uno::Sequence< uno::Type > SAL_CALL ScAccessibleContextBase::getTypes()
        throw (uno::RuntimeException)
{
    return comphelper::concatSequences(ScAccessibleContextBaseWeakImpl::getTypes(), ScAccessibleContextBaseImplEvent::getTypes());
}

namespace
{
    class theScAccessibleContextBaseImplementationId : public rtl::Static< UnoTunnelIdInit, theScAccessibleContextBaseImplementationId > {};
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleContextBase::getImplementationId(void)
    throw (uno::RuntimeException)
{
    return theScAccessibleContextBaseImplementationId::get().getSeq();
}

//=====  internal  ============================================================

::rtl::OUString SAL_CALL
    ScAccessibleContextBase::createAccessibleDescription(void)
    throw (uno::RuntimeException)
{
    OSL_FAIL("should be implemented in the abrevated class");
    return rtl::OUString();
}

::rtl::OUString SAL_CALL
    ScAccessibleContextBase::createAccessibleName(void)
    throw (uno::RuntimeException)
{
    OSL_FAIL("should be implemented in the abrevated class");
    return rtl::OUString();
}

void ScAccessibleContextBase::CommitChange(const AccessibleEventObject& rEvent) const
{
    if (mnClientId)
        comphelper::AccessibleEventNotifier::addEvent( mnClientId, rEvent );
}

void ScAccessibleContextBase::ChangeName()
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::NAME_CHANGED;
    aEvent.Source = uno::Reference< XAccessibleContext >(const_cast<ScAccessibleContextBase*>(this));
    aEvent.OldValue <<= msName;

    msName = rtl::OUString(); // reset the name so it will be hold again
    getAccessibleName(); // create the new name

    aEvent.NewValue <<= msName;

    CommitChange(aEvent);
}

void ScAccessibleContextBase::CommitFocusGained() const
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::STATE_CHANGED;
    aEvent.Source = uno::Reference< XAccessibleContext >(const_cast<ScAccessibleContextBase*>(this));
    aEvent.NewValue <<= AccessibleStateType::FOCUSED;

    CommitChange(aEvent);

    ::vcl::unohelper::NotifyAccessibleStateEventGlobally(aEvent);
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

Rectangle ScAccessibleContextBase::GetBoundingBoxOnScreen(void) const
        throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented");
    return Rectangle();
}

Rectangle ScAccessibleContextBase::GetBoundingBox(void) const
        throw (uno::RuntimeException)
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
