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


#include "AccessibleTreeNode.hxx"

#include "taskpane/TaskPaneTreeNode.hxx"
#include "taskpane/ControlContainer.hxx"

#include "sdresid.hxx"
#include "accessibility.hrc"
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <comphelper/accessibleeventnotifier.hxx>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <svtools/colorcfg.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using namespace ::sd::toolpanel;

namespace accessibility {



//===== AccessibleTreeNode =============================================

AccessibleTreeNode::AccessibleTreeNode(
    ::sd::toolpanel::TreeNode& rNode,
    const OUString& rsName,
    const OUString& rsDescription,
    sal_Int16 eRole)
    : AccessibleTreeNodeBase(MutexOwner::maMutex),
      mxParent(NULL),
      mrTreeNode(rNode),
      mrStateSet(new ::utl::AccessibleStateSetHelper()),
      msName(rsName),
      msDescription(rsDescription),
      meRole(eRole),
      mnClientId(0)
{
    ::Window* pWindow = mrTreeNode.GetWindow();
    if (pWindow != NULL)
    {
        ::Window* pParentWindow = pWindow->GetAccessibleParentWindow();
        if (pParentWindow != NULL && pParentWindow != pWindow)
            mxParent = pParentWindow->GetAccessible();
    }
    CommonConstructor();
}




void AccessibleTreeNode::CommonConstructor (void)
{
    UpdateStateSet();

    Link aStateChangeLink (LINK(this,AccessibleTreeNode,StateChangeListener));
    mrTreeNode.AddStateChangeListener(aStateChangeLink);

    if (mrTreeNode.GetWindow() != NULL)
    {
        Link aWindowEventLink (LINK(this,AccessibleTreeNode,WindowEventListener));
        mrTreeNode.GetWindow()->AddEventListener(aWindowEventLink);
    }
}




AccessibleTreeNode::~AccessibleTreeNode (void)
{
    OSL_ASSERT(IsDisposed());
}




void AccessibleTreeNode::FireAccessibleEvent (
    short nEventId,
    const uno::Any& rOldValue,
    const uno::Any& rNewValue )
{
    if (mnClientId != 0)
    {
        AccessibleEventObject aEventObject;

        aEventObject.Source = Reference<XWeak>(this);
        aEventObject.EventId = nEventId;
        aEventObject.NewValue = rNewValue;
        aEventObject.OldValue = rOldValue;

        comphelper::AccessibleEventNotifier::addEvent (mnClientId, aEventObject);
    }
}




void SAL_CALL AccessibleTreeNode::disposing (void)
{
    // We are still listening to the tree node and its window.  Both
    // probably are by now more or less dead and we must not call them to
    // unregister.

    if (mnClientId != 0)
    {
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( mnClientId, *this );
        mnClientId = 0;
    }
}




//=====  XAccessible  =========================================================

Reference<XAccessibleContext > SAL_CALL
    AccessibleTreeNode::getAccessibleContext (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();
    return this;
}




//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL AccessibleTreeNode::getAccessibleChildCount (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    return mrTreeNode.GetControlContainer().GetControlCount();
}




Reference<XAccessible > SAL_CALL
    AccessibleTreeNode::getAccessibleChild (sal_Int32 nIndex)
    throw (lang::IndexOutOfBoundsException, RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    if (nIndex<0 || (sal_uInt32)nIndex>=mrTreeNode.GetControlContainer().GetControlCount())
        throw lang::IndexOutOfBoundsException();

    Reference<XAccessible> xChild;

    ::sd::toolpanel::TreeNode* pNode = mrTreeNode.GetControlContainer().GetControl(nIndex);
    if (pNode != NULL)
        xChild = pNode->GetAccessibleObject();

    return xChild;
}




Reference<XAccessible > SAL_CALL AccessibleTreeNode::getAccessibleParent (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    return mxParent;
}




sal_Int32 SAL_CALL AccessibleTreeNode::getAccessibleIndexInParent (void)
    throw (uno::RuntimeException)
{
    OSL_ASSERT(getAccessibleParent().is());
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    sal_Int32 nIndexInParent(-1);


    Reference<XAccessibleContext> xParentContext (getAccessibleParent()->getAccessibleContext());
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

    return nIndexInParent;
}




sal_Int16 SAL_CALL AccessibleTreeNode::getAccessibleRole (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return meRole;
}




OUString SAL_CALL AccessibleTreeNode::getAccessibleDescription (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return msDescription;
}




OUString SAL_CALL AccessibleTreeNode::getAccessibleName (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return msName;
}




Reference<XAccessibleRelationSet> SAL_CALL
    AccessibleTreeNode::getAccessibleRelationSet (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    return Reference<XAccessibleRelationSet>();
}




Reference<XAccessibleStateSet > SAL_CALL
    AccessibleTreeNode::getAccessibleStateSet (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    return mrStateSet.get();
}




void AccessibleTreeNode::UpdateStateSet (void)
{
    if (mrTreeNode.IsExpandable())
    {
        UpdateState(AccessibleStateType::EXPANDABLE, true);
        UpdateState(AccessibleStateType::EXPANDED, mrTreeNode.IsExpanded());
    }

    UpdateState(AccessibleStateType::FOCUSABLE, true);

    ::Window* pWindow = mrTreeNode.GetWindow();
    if (pWindow != NULL)
    {
        UpdateState(AccessibleStateType::ENABLED, pWindow->IsEnabled());
        UpdateState(AccessibleStateType::FOCUSED, pWindow->HasFocus());
        UpdateState(AccessibleStateType::VISIBLE, pWindow->IsVisible());
        UpdateState(AccessibleStateType::SHOWING, pWindow->IsReallyVisible());
    }
}




void AccessibleTreeNode::UpdateState(
    sal_Int16 aState,
    bool bValue)
{
    if ((mrStateSet->contains(aState)!=sal_False) != bValue)
    {
        if (bValue)
        {
            mrStateSet->AddState(aState);
            FireAccessibleEvent(AccessibleEventId::STATE_CHANGED, Any(),Any(aState));
        }
        else
        {
            mrStateSet->RemoveState(aState);
            FireAccessibleEvent(AccessibleEventId::STATE_CHANGED, Any(aState),Any());
        }
    }
}




lang::Locale SAL_CALL AccessibleTreeNode::getLocale (void)
    throw (IllegalAccessibleComponentStateException,
        RuntimeException)
{
    ThrowIfDisposed ();
    Reference<XAccessibleContext> xParentContext;
    Reference<XAccessible> xParent (getAccessibleParent());
    if (xParent.is())
        xParentContext = xParent->getAccessibleContext();

    if (xParentContext.is())
        return xParentContext->getLocale();
    else
        // Strange, no parent!  Anyway, return the default locale.
        return Application::GetSettings().GetLanguageTag().getLocale();
}




void SAL_CALL AccessibleTreeNode::addAccessibleEventListener(
    const Reference<XAccessibleEventListener >& rxListener)
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
            if (mnClientId != 0)
                comphelper::AccessibleEventNotifier::addEventListener(mnClientId, rxListener);
        }
    }
}




void SAL_CALL AccessibleTreeNode::removeAccessibleEventListener(
    const Reference<XAccessibleEventListener >& rxListener)
    throw (RuntimeException)
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
            if (mnClientId != 0)
            {
                comphelper::AccessibleEventNotifier::revokeClient( mnClientId );
                mnClientId = 0;
            }
        }
    }
}




//===== XAccessibleComponent ==================================================

sal_Bool SAL_CALL AccessibleTreeNode::containsPoint (const awt::Point& aPoint)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    const awt::Rectangle aBBox (getBounds());
    return (aPoint.X >= 0)
        && (aPoint.X < aBBox.Width)
        && (aPoint.Y >= 0)
        && (aPoint.Y < aBBox.Height);
}




Reference<XAccessible> SAL_CALL
    AccessibleTreeNode::getAccessibleAtPoint (const awt::Point& aPoint)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    Reference<XAccessible> xChildAtPoint;
    const SolarMutexGuard aSolarGuard;

    sal_Int32 nChildCount = getAccessibleChildCount();
    for (sal_Int32 nIndex=0; nIndex<nChildCount; ++nIndex)
    {
        Reference<XAccessibleComponent> xChildComponent(
            getAccessibleChild(nIndex), UNO_QUERY);
        if (xChildComponent.is())
        {
            awt::Point aChildPoint(aPoint);
            awt::Point aChildOrigin(xChildComponent->getLocation());
            aChildPoint.X -= aChildOrigin.X;
            aChildPoint.Y -= aChildOrigin.Y;
            if (xChildComponent->containsPoint(aChildPoint))
            {
                xChildAtPoint = getAccessibleChild(nIndex);
                break;
            }
        }
    }

    return xChildAtPoint;
}




awt::Rectangle SAL_CALL AccessibleTreeNode::getBounds (void)
    throw (RuntimeException)
{
    ThrowIfDisposed ();

    awt::Rectangle aBBox;

    ::Window* pWindow = mrTreeNode.GetWindow();
    if (pWindow != NULL)
    {
        Point aPosition;
        if (mxParent.is())
        {
            aPosition = pWindow->OutputToAbsoluteScreenPixel(Point(0,0));
            Reference<XAccessibleComponent> xParentComponent (
                mxParent->getAccessibleContext(), UNO_QUERY);
            if (xParentComponent.is())
            {
                awt::Point aParentPosition (xParentComponent->getLocationOnScreen());
                aPosition.X() -= aParentPosition.X;
                aPosition.Y() -= aParentPosition.Y;
            }
        }
        else
            aPosition = pWindow->GetPosPixel();
        aBBox.X = aPosition.X();
        aBBox.Y = aPosition.Y();

        Size aSize (pWindow->GetSizePixel());
        aBBox.Width = aSize.Width();
        aBBox.Height = aSize.Height();
    }

    return aBBox;
}




awt::Point SAL_CALL AccessibleTreeNode::getLocation (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const awt::Rectangle aBBox (getBounds());
    return awt::Point(aBBox.X,aBBox.Y);
}




/** Calculate the location on screen from the parent's location on screen
    and our own relative location.
*/
awt::Point SAL_CALL AccessibleTreeNode::getLocationOnScreen()
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    awt::Point aLocationOnScreen;

    ::Window* pWindow = mrTreeNode.GetWindow();
    if (pWindow != NULL)
    {
        Point aPoint (pWindow->OutputToAbsoluteScreenPixel(Point(0,0)));
        aLocationOnScreen.X = aPoint.X();
        aLocationOnScreen.Y = aPoint.Y();
    }

    return aLocationOnScreen;
}




awt::Size SAL_CALL AccessibleTreeNode::getSize (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const awt::Rectangle aBBox (getBounds());
    return awt::Size(aBBox.Width,aBBox.Height);
}




void SAL_CALL AccessibleTreeNode::grabFocus (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    if (mrTreeNode.GetWindow() != NULL)
        mrTreeNode.GetWindow()->GrabFocus();
}




sal_Int32 SAL_CALL AccessibleTreeNode::getForeground (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    svtools::ColorConfig aColorConfig;
    sal_uInt32 nColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
    return static_cast<sal_Int32>(nColor);
}




sal_Int32 SAL_CALL AccessibleTreeNode::getBackground (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}




//=====  XServiceInfo  ========================================================

OUString SAL_CALL
       AccessibleTreeNode::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString("AccessibleTreeNode");
}




sal_Bool SAL_CALL
     AccessibleTreeNode::supportsService (const OUString& sServiceName)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    //  Iterate over all supported service names and return true if on of them
    //  matches the given name.
    uno::Sequence< OUString> aSupportedServices (
        getSupportedServiceNames ());
    for (int i=0; i<aSupportedServices.getLength(); i++)
        if (sServiceName == aSupportedServices[i])
            return sal_True;
    return sal_False;
}




uno::Sequence< OUString> SAL_CALL
       AccessibleTreeNode::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    static const OUString sServiceNames[2] = {
            OUString("com.sun.star.accessibility.Accessible"),
            OUString("com.sun.star.accessibility.AccessibleContext"),
    };
    return uno::Sequence<OUString> (sServiceNames, 2);
}




void AccessibleTreeNode::ThrowIfDisposed (void)
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        OSL_TRACE ("Calling disposed object. Throwing exception:");
        throw lang::DisposedException ("object has been already disposed",
            static_cast<uno::XWeak*>(this));
    }
}



sal_Bool AccessibleTreeNode::IsDisposed (void)
{
    return (rBHelper.bDisposed || rBHelper.bInDispose);
}




IMPL_LINK(AccessibleTreeNode, StateChangeListener, TreeNodeStateChangeEvent*, pEvent)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        return 1; // mrTreeNode is probably dead

    OSL_ASSERT(pEvent!=NULL);
    OSL_ASSERT(&pEvent->mrSource==&mrTreeNode);

    switch(pEvent->meEventId)
    {
        case EID_CHILD_ADDED:
            if (pEvent->mpChild != NULL)
                FireAccessibleEvent(AccessibleEventId::CHILD,
                    Any(),
                    Any(pEvent->mpChild->GetAccessibleObject()));
            else
                FireAccessibleEvent(AccessibleEventId::INVALIDATE_ALL_CHILDREN,Any(),Any());
            break;

        case EID_ALL_CHILDREN_REMOVED:
            FireAccessibleEvent(AccessibleEventId::INVALIDATE_ALL_CHILDREN,Any(),Any());
            break;

        case EID_EXPANSION_STATE_CHANGED:
        case EID_FOCUSED_STATE_CHANGED:
        case EID_SHOWING_STATE_CHANGED:
            UpdateStateSet();
            break;
    }
    return 1;
}




IMPL_LINK(AccessibleTreeNode, WindowEventListener, VclWindowEvent*, pEvent)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        return 1; // mrTreeNode is probably dead

    switch (pEvent->GetId())
    {
        case VCLEVENT_WINDOW_HIDE:
            // This event may be sent while the window is destroyed so do
            // not call UpdateStateSet() which calls back to the window but
            // just set the two states VISIBLE and SHOWING to false.
            UpdateState(AccessibleStateType::VISIBLE, false);
            UpdateState(AccessibleStateType::SHOWING, false);
            break;

        case VCLEVENT_WINDOW_SHOW:
        case VCLEVENT_WINDOW_DATACHANGED:
            UpdateStateSet();
            break;

        case VCLEVENT_WINDOW_MOVE:
        case VCLEVENT_WINDOW_RESIZE:
            FireAccessibleEvent(AccessibleEventId::BOUNDRECT_CHANGED,Any(),Any());
            break;

        case VCLEVENT_WINDOW_GETFOCUS:
        case VCLEVENT_WINDOW_LOSEFOCUS:
            UpdateStateSet();
            break;
    }
    return 1;
}

} // end of namespace ::accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
