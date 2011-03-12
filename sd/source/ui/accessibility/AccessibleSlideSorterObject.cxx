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

#include "AccessibleSlideSorterObject.hxx"

#include "SlideSorter.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsFocusManager.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsPageObjectLayouter.hxx"
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <unotools/accessiblestatesethelper.hxx>

#include "sdpage.hxx"
#include "sdresid.hxx"
#include <vcl/svapp.hxx>

#include "glob.hrc"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;


namespace accessibility {


AccessibleSlideSorterObject::AccessibleSlideSorterObject(
    const Reference<XAccessible>& rxParent,
    ::sd::slidesorter::SlideSorter& rSlideSorter,
    sal_uInt16 nPageNumber)
    : AccessibleSlideSorterObjectBase(::sd::MutexOwner::maMutex),
      mxParent(rxParent),
      mnPageNumber(nPageNumber),
      mrSlideSorter(rSlideSorter),
      mnClientId(0)
{
}




AccessibleSlideSorterObject::~AccessibleSlideSorterObject (void)
{
    if ( ! IsDisposed())
        dispose();
}




sal_uInt16 AccessibleSlideSorterObject::GetPageNumber (void) const
{
    return mnPageNumber;
}




void AccessibleSlideSorterObject::FireAccessibleEvent (
    short nEventId,
    const uno::Any& rOldValue,
    const uno::Any& rNewValue)
{
    if (mnClientId != 0)
    {
        AccessibleEventObject aEventObject;

        aEventObject.Source = Reference<XWeak>(this);
        aEventObject.EventId = nEventId;
        aEventObject.NewValue = rNewValue;
        aEventObject.OldValue = rOldValue;

        comphelper::AccessibleEventNotifier::addEvent(mnClientId, aEventObject);
    }
}




void SAL_CALL AccessibleSlideSorterObject::disposing (void)
{
    const SolarMutexGuard aSolarGuard;

    // Send a disposing to all listeners.
    if (mnClientId != 0)
    {
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing(mnClientId, *this);
        mnClientId =  0;
    }
}



//===== XAccessible ===========================================================

Reference<XAccessibleContext> SAL_CALL
    AccessibleSlideSorterObject::getAccessibleContext (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return this;
}



//===== XAccessibleContext ====================================================

sal_Int32 SAL_CALL AccessibleSlideSorterObject::getAccessibleChildCount (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return 0;
}




Reference<XAccessible> SAL_CALL AccessibleSlideSorterObject::getAccessibleChild (sal_Int32 )
    throw (lang::IndexOutOfBoundsException, RuntimeException)
{
    ThrowIfDisposed();
    throw lang::IndexOutOfBoundsException();
}




Reference<XAccessible> SAL_CALL AccessibleSlideSorterObject::getAccessibleParent (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return mxParent;
}




sal_Int32 SAL_CALL AccessibleSlideSorterObject::getAccessibleIndexInParent()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    sal_Int32 nIndexInParent(-1);

    if (mxParent.is())
    {
        Reference<XAccessibleContext> xParentContext (mxParent->getAccessibleContext());
        if (xParentContext.is())
        {
            sal_Int32 nChildCount (xParentContext->getAccessibleChildCount());
            for (sal_Int32 i=0; i<nChildCount; ++i)
                if (xParentContext->getAccessibleChild(i).get()
                    == static_cast<XAccessible*>(this))
                {
                    nIndexInParent = i;
                    break;
                }
        }
    }

    return nIndexInParent;
}




sal_Int16 SAL_CALL AccessibleSlideSorterObject::getAccessibleRole (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    static sal_Int16 nRole = AccessibleRole::LIST_ITEM;
    return nRole;
}




::rtl::OUString SAL_CALL AccessibleSlideSorterObject::getAccessibleDescription (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return String(SdResId(STR_PAGE));
}




::rtl::OUString SAL_CALL AccessibleSlideSorterObject::getAccessibleName (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    SdPage* pPage = GetPage();
    if (pPage != NULL)
        return pPage->GetName();
    else
        return String();
}




Reference<XAccessibleRelationSet> SAL_CALL
    AccessibleSlideSorterObject::getAccessibleRelationSet (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return Reference<XAccessibleRelationSet>();
}




Reference<XAccessibleStateSet> SAL_CALL
    AccessibleSlideSorterObject::getAccessibleStateSet (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ::utl::AccessibleStateSetHelper* pStateSet = new ::utl::AccessibleStateSetHelper();

    if (mxParent.is())
    {
        // Unconditional states.
        pStateSet->AddState(AccessibleStateType::SELECTABLE);
        pStateSet->AddState(AccessibleStateType::FOCUSABLE);
        pStateSet->AddState(AccessibleStateType::ENABLED);
        pStateSet->AddState(AccessibleStateType::VISIBLE);
        pStateSet->AddState(AccessibleStateType::SHOWING);
        pStateSet->AddState(AccessibleStateType::ACTIVE);
        pStateSet->AddState(AccessibleStateType::SENSITIVE);

        // Conditional states.
        if (mrSlideSorter.GetController().GetPageSelector().IsPageSelected(mnPageNumber))
            pStateSet->AddState(AccessibleStateType::SELECTED);
        if (mrSlideSorter.GetController().GetFocusManager().GetFocusedPageIndex() == mnPageNumber)
            if (mrSlideSorter.GetController().GetFocusManager().IsFocusShowing())
                pStateSet->AddState(AccessibleStateType::FOCUSED);
    }

    return pStateSet;
}




lang::Locale SAL_CALL AccessibleSlideSorterObject::getLocale (void)
    throw (IllegalAccessibleComponentStateException,
        RuntimeException)
{
    ThrowIfDisposed();
    // Delegate request to parent.
    if (mxParent.is())
    {
        Reference<XAccessibleContext> xParentContext (mxParent->getAccessibleContext());
        if (xParentContext.is())
            return xParentContext->getLocale ();
    }

    //  No locale and no parent.  Therefore throw exception to indicate this
    //  cluelessness.
    throw IllegalAccessibleComponentStateException();
}





//===== XAccessibleEventBroadcaster ===========================================

void SAL_CALL AccessibleSlideSorterObject::addEventListener(
    const Reference<XAccessibleEventListener>& rxListener)
    throw (RuntimeException)
{
    if (rxListener.is())
    {
        const osl::MutexGuard aGuard(maMutex);

        if (IsDisposed())
        {
            uno::Reference<uno::XInterface> x ((lang::XComponent *)this, uno::UNO_QUERY);
            rxListener->disposing (lang::EventObject (x));
        }
        else
        {
            if (mnClientId == 0)
                mnClientId = comphelper::AccessibleEventNotifier::registerClient();
            comphelper::AccessibleEventNotifier::addEventListener(mnClientId, rxListener);
        }
    }
}




void SAL_CALL AccessibleSlideSorterObject::removeEventListener(
    const Reference<XAccessibleEventListener>& rxListener)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    if (rxListener.is())
    {
        const osl::MutexGuard aGuard(maMutex);

        sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( mnClientId, rxListener );
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




//===== XAccessibleComponent ==================================================

sal_Bool SAL_CALL AccessibleSlideSorterObject::containsPoint(const awt::Point& aPoint)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const awt::Size aSize (getSize());
    return (aPoint.X >= 0)
        && (aPoint.X < aSize.Width)
        && (aPoint.Y >= 0)
        && (aPoint.Y < aSize.Height);
}




Reference<XAccessible> SAL_CALL
    AccessibleSlideSorterObject::getAccessibleAtPoint(const awt::Point& )
    throw (uno::RuntimeException)
{
    return NULL;
}




awt::Rectangle SAL_CALL AccessibleSlideSorterObject::getBounds (void)
    throw (RuntimeException)
{
    ThrowIfDisposed ();

    const SolarMutexGuard aSolarGuard;

    Rectangle aBBox (
        mrSlideSorter.GetView().GetLayouter().GetPageObjectLayouter()->GetBoundingBox(
            mrSlideSorter.GetModel().GetPageDescriptor(mnPageNumber),
            ::sd::slidesorter::view::PageObjectLayouter::PageObject,
            ::sd::slidesorter::view::PageObjectLayouter::WindowCoordinateSystem));

    if (mxParent.is())
    {
        Reference<XAccessibleComponent> xParentComponent(mxParent->getAccessibleContext(), UNO_QUERY);
        if (xParentComponent.is())
        {
            awt::Rectangle aParentBBox (xParentComponent->getBounds());
            aBBox.Intersection(Rectangle(
                aParentBBox.X,
                aParentBBox.Y,
                aParentBBox.Width,
                aParentBBox.Height));
        }
    }

    return awt::Rectangle(
        aBBox.Left(),
        aBBox.Top(),
        aBBox.GetWidth(),
        aBBox.GetHeight());
}




awt::Point SAL_CALL AccessibleSlideSorterObject::getLocation ()
    throw (RuntimeException)
{
    ThrowIfDisposed ();
    const awt::Rectangle aBBox (getBounds());
    return awt::Point(aBBox.X, aBBox.Y);
}




awt::Point SAL_CALL AccessibleSlideSorterObject::getLocationOnScreen (void)
    throw (RuntimeException)
{
    ThrowIfDisposed ();

    const SolarMutexGuard aSolarGuard;

    awt::Point aLocation (getLocation());

    if (mxParent.is())
    {
        Reference<XAccessibleComponent> xParentComponent(mxParent->getAccessibleContext(),UNO_QUERY);
        if (xParentComponent.is())
        {
            const awt::Point aParentLocationOnScreen(xParentComponent->getLocationOnScreen());
            aLocation.X += aParentLocationOnScreen.X;
            aLocation.Y += aParentLocationOnScreen.Y;
        }
    }

    return aLocation;
}




awt::Size SAL_CALL AccessibleSlideSorterObject::getSize (void)
    throw (RuntimeException)
{
    ThrowIfDisposed ();
    const awt::Rectangle aBBox (getBounds());
    return awt::Size(aBBox.Width,aBBox.Height);
}




void SAL_CALL AccessibleSlideSorterObject::grabFocus (void)
    throw (RuntimeException)
{
    // nothing to do
}




sal_Int32 SAL_CALL AccessibleSlideSorterObject::getForeground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    svtools::ColorConfig aColorConfig;
    sal_uInt32 nColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
    return static_cast<sal_Int32>(nColor);
}




sal_Int32 SAL_CALL AccessibleSlideSorterObject::getBackground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}





//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
       AccessibleSlideSorterObject::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleSlideSorterObject"));
}




sal_Bool SAL_CALL
     AccessibleSlideSorterObject::supportsService (const OUString& sServiceName)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    //  Iterate over all supported service names and return true if on of them
    //  matches the given name.
    uno::Sequence< ::rtl::OUString> aSupportedServices (
        getSupportedServiceNames ());
    for (int i=0; i<aSupportedServices.getLength(); i++)
        if (sServiceName == aSupportedServices[i])
            return sal_True;
    return sal_False;
}




uno::Sequence< ::rtl::OUString> SAL_CALL
       AccessibleSlideSorterObject::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    static const OUString sServiceNames[2] = {
        OUString(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.accessibility.Accessible")),
        OUString(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.accessibility.AccessibleContext"))
    };
    return uno::Sequence<OUString> (sServiceNames, 2);
}




void AccessibleSlideSorterObject::ThrowIfDisposed (void)
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        OSL_TRACE ("Calling disposed object. Throwing exception:");
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM("object has been already disposed")),
            static_cast<uno::XWeak*>(this));
    }
}



sal_Bool AccessibleSlideSorterObject::IsDisposed (void)
{
    return (rBHelper.bDisposed || rBHelper.bInDispose);
}




SdPage* AccessibleSlideSorterObject::GetPage (void) const
{
    ::sd::slidesorter::model::SharedPageDescriptor pDescriptor(
        mrSlideSorter.GetModel().GetPageDescriptor(mnPageNumber));
    if (pDescriptor.get() != NULL)
        return pDescriptor->GetPage();
    else
        return NULL;
}


} // end of namespace ::accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
