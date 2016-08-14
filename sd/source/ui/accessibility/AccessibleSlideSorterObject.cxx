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
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblestatesethelper.hxx>

#include "sdpage.hxx"
#include "sdresid.hxx"
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include "glob.hrc"

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

AccessibleSlideSorterObject::~AccessibleSlideSorterObject()
{
    if ( ! IsDisposed())
        dispose();
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

void SAL_CALL AccessibleSlideSorterObject::disposing()
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
    AccessibleSlideSorterObject::getAccessibleContext()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return this;
}

//===== XAccessibleContext ====================================================

sal_Int32 SAL_CALL AccessibleSlideSorterObject::getAccessibleChildCount()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return 0;
}

Reference<XAccessible> SAL_CALL AccessibleSlideSorterObject::getAccessibleChild (sal_Int32 )
    throw (lang::IndexOutOfBoundsException, RuntimeException, std::exception)
{
    ThrowIfDisposed();
    throw lang::IndexOutOfBoundsException();
}

Reference<XAccessible> SAL_CALL AccessibleSlideSorterObject::getAccessibleParent()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return mxParent;
}

sal_Int32 SAL_CALL AccessibleSlideSorterObject::getAccessibleIndexInParent()
    throw (uno::RuntimeException, std::exception)
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

sal_Int16 SAL_CALL AccessibleSlideSorterObject::getAccessibleRole()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    //set Role = Shape
    //static sal_Int16 nRole = AccessibleRole::LIST_ITEM;
    static sal_Int16 nRole = AccessibleRole::SHAPE;
    return nRole;
}

OUString SAL_CALL AccessibleSlideSorterObject::getAccessibleDescription()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return SD_RESSTR(STR_PAGE);
}

OUString SAL_CALL AccessibleSlideSorterObject::getAccessibleName()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    SdPage* pPage = GetPage();
    if (pPage != nullptr)
        return pPage->GetName();
    else
        return OUString();
}

Reference<XAccessibleRelationSet> SAL_CALL
    AccessibleSlideSorterObject::getAccessibleRelationSet()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    return Reference<XAccessibleRelationSet>();
}

Reference<XAccessibleStateSet> SAL_CALL
    AccessibleSlideSorterObject::getAccessibleStateSet()
    throw (uno::RuntimeException, std::exception)
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

lang::Locale SAL_CALL AccessibleSlideSorterObject::getLocale()
    throw (IllegalAccessibleComponentStateException,
        RuntimeException, std::exception)
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

void SAL_CALL AccessibleSlideSorterObject::addAccessibleEventListener(
    const Reference<XAccessibleEventListener>& rxListener)
    throw (RuntimeException, std::exception)
{
    if (rxListener.is())
    {
        const osl::MutexGuard aGuard(maMutex);

        if (IsDisposed())
        {
            uno::Reference<uno::XInterface> x (static_cast<lang::XComponent *>(this), uno::UNO_QUERY);
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

void SAL_CALL AccessibleSlideSorterObject::removeAccessibleEventListener(
    const Reference<XAccessibleEventListener>& rxListener)
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
{
    return nullptr;
}

awt::Rectangle SAL_CALL AccessibleSlideSorterObject::getBounds()
    throw (RuntimeException, std::exception)
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
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed ();
    const awt::Rectangle aBBox (getBounds());
    return awt::Point(aBBox.X, aBBox.Y);
}

awt::Point SAL_CALL AccessibleSlideSorterObject::getLocationOnScreen()
    throw (RuntimeException, std::exception)
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

awt::Size SAL_CALL AccessibleSlideSorterObject::getSize()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed ();
    const awt::Rectangle aBBox (getBounds());
    return awt::Size(aBBox.Width,aBBox.Height);
}

void SAL_CALL AccessibleSlideSorterObject::grabFocus()
    throw (RuntimeException, std::exception)
{
    // nothing to do
}

sal_Int32 SAL_CALL AccessibleSlideSorterObject::getForeground()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed ();
    svtools::ColorConfig aColorConfig;
    sal_uInt32 nColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL AccessibleSlideSorterObject::getBackground()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed ();
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

// XServiceInfo
OUString SAL_CALL
       AccessibleSlideSorterObject::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("AccessibleSlideSorterObject");
}

sal_Bool SAL_CALL AccessibleSlideSorterObject::supportsService (const OUString& sServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, sServiceName);
}

uno::Sequence< OUString> SAL_CALL
       AccessibleSlideSorterObject::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed ();

    return uno::Sequence<OUString> {
        OUString("com.sun.star.accessibility.Accessible"),
        OUString("com.sun.star.accessibility.AccessibleContext")
    };
}

void AccessibleSlideSorterObject::ThrowIfDisposed()
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        OSL_TRACE ("Calling disposed object. Throwing exception:");
        throw lang::DisposedException ("object has been already disposed",
            static_cast<uno::XWeak*>(this));
    }
}

bool AccessibleSlideSorterObject::IsDisposed()
{
    return (rBHelper.bDisposed || rBHelper.bInDispose);
}

SdPage* AccessibleSlideSorterObject::GetPage() const
{
    ::sd::slidesorter::model::SharedPageDescriptor pDescriptor(
        mrSlideSorter.GetModel().GetPageDescriptor(mnPageNumber));
    if (pDescriptor.get() != nullptr)
        return pDescriptor->GetPage();
    else
        return nullptr;
}

} // end of namespace ::accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
