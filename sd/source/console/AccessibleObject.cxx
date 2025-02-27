/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include "AccessibleObject.hxx"
#include "AccessibleFocusManager.hxx"

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>


AccessibleObject::AccessibleObject(
    const sal_Int16 nRole,
    OUString sName)
    : msName(std::move(sName)),
      mnRole(nRole),
      mnStateSet(0),
      mbIsFocused(false)
{
}

void AccessibleObject::LateInitialization()
{
    AccessibleFocusManager::Instance()->AddFocusableObject(this);
}

void AccessibleObject::SetWindow (
    const Reference<awt::XWindow>& rxContentWindow,
    const Reference<awt::XWindow>& rxBorderWindow)
{
    Reference<awt::XWindow2> xContentWindow (rxContentWindow, UNO_QUERY);

    if (mxContentWindow.get() == xContentWindow.get())
        return;

    if (mxContentWindow.is())
    {
        mxContentWindow->removeWindowListener(this);
    }

    mxContentWindow = std::move(xContentWindow);
    mxBorderWindow.set(rxBorderWindow, UNO_QUERY);

    if (mxContentWindow.is())
    {
        mxContentWindow->addWindowListener(this);
    }

    UpdateStateSet();
}

void AccessibleObject::SetAccessibleParent (
    const Reference<XAccessible>& rxAccessibleParent)
{
    mxParentAccessible = rxAccessibleParent;
}

void SAL_CALL AccessibleObject::disposing()
{
    OAccessibleComponentHelper::disposing();

    AccessibleFocusManager::Instance()->RemoveFocusableObject(this);
    SetWindow(nullptr, nullptr);
}

//----- XAccessible -------------------------------------------------------

Reference<XAccessibleContext> SAL_CALL
    AccessibleObject::getAccessibleContext()
{
    ThrowIfDisposed();

    return this;
}

//-----  XAccessibleContext  ----------------------------------------------

sal_Int64 SAL_CALL AccessibleObject::getAccessibleChildCount()
{
    ThrowIfDisposed();

    return maChildren.size();
}

Reference<XAccessible> SAL_CALL
    AccessibleObject::getAccessibleChild (sal_Int64 nIndex)
{
    ThrowIfDisposed();

    if (nIndex<0 || o3tl::make_unsigned(nIndex)>=maChildren.size())
        throw lang::IndexOutOfBoundsException(u"invalid child index"_ustr, static_cast<uno::XWeak*>(this));

    return maChildren[nIndex];
}

Reference<XAccessible> SAL_CALL
    AccessibleObject::getAccessibleParent()
{
    ThrowIfDisposed();

    return mxParentAccessible;
}

sal_Int64 SAL_CALL
    AccessibleObject::getAccessibleIndexInParent()
{
    ThrowIfDisposed();

    const Reference<XAccessible> xThis (this);
    if (mxParentAccessible.is())
    {
        const Reference<XAccessibleContext> xContext (mxParentAccessible->getAccessibleContext());
        for (sal_Int64 nIndex = 0, nCount=xContext->getAccessibleChildCount();
             nIndex<nCount;
             ++nIndex)
        {
            if (xContext->getAccessibleChild(nIndex) == xThis)
                return nIndex;
        }
    }

    return 0;
}

sal_Int16 SAL_CALL
    AccessibleObject::getAccessibleRole()
{
    ThrowIfDisposed();

    return mnRole;
}

OUString SAL_CALL
    AccessibleObject::getAccessibleDescription()
{
    ThrowIfDisposed();

    return OUString();
}

OUString SAL_CALL
    AccessibleObject::getAccessibleName()
{
    ThrowIfDisposed();

    return msName;
}

Reference<XAccessibleRelationSet> SAL_CALL
    AccessibleObject::getAccessibleRelationSet()
{
    ThrowIfDisposed();

    return nullptr;
}

sal_Int64 SAL_CALL
    AccessibleObject::getAccessibleStateSet()
{
    ThrowIfDisposed();

    return mnStateSet;
}

lang::Locale SAL_CALL
    AccessibleObject::getLocale()
{
    ThrowIfDisposed();

    if (mxParentAccessible.is())
    {
        Reference<XAccessibleContext> xParentContext (mxParentAccessible->getAccessibleContext());
        if (xParentContext.is())
            return xParentContext->getLocale();
    }
    return css::lang::Locale();
}

//-----  XAccessibleComponent  ------------------------------------------------

Reference<XAccessible> SAL_CALL
    AccessibleObject::getAccessibleAtPoint (const awt::Point&)
{
    ThrowIfDisposed();

    return Reference<XAccessible>();
}

awt::Rectangle AccessibleObject::implGetBounds()
{
    const awt::Point aLocation (GetRelativeLocation());
    const awt::Size aSize (GetSize());

    return awt::Rectangle (aLocation.X, aLocation.Y, aSize.Width, aSize.Height);
}

void SAL_CALL AccessibleObject::grabFocus()
{
    ThrowIfDisposed();
    if (mxBorderWindow.is())
        mxBorderWindow->setFocus();
    else if (mxContentWindow.is())
        mxContentWindow->setFocus();
}

sal_Int32 SAL_CALL AccessibleObject::getForeground()
{
    ThrowIfDisposed();

    return 0x00ffffff;
}

sal_Int32 SAL_CALL AccessibleObject::getBackground()
{
    ThrowIfDisposed();

    return 0x00000000;
}

//----- XWindowListener ---------------------------------------------------

void SAL_CALL AccessibleObject::windowResized (
    const css::awt::WindowEvent&)
{
    FireAccessibleEvent(AccessibleEventId::BOUNDRECT_CHANGED, Any(), Any());
}

void SAL_CALL AccessibleObject::windowMoved (
    const css::awt::WindowEvent&)
{
    FireAccessibleEvent(AccessibleEventId::BOUNDRECT_CHANGED, Any(), Any());
}

void SAL_CALL AccessibleObject::windowShown (
    const css::lang::EventObject&)
{
    UpdateStateSet();
}

void SAL_CALL AccessibleObject::windowHidden (
    const css::lang::EventObject&)
{
    UpdateStateSet();
}

//----- XEventListener --------------------------------------------------------

void SAL_CALL AccessibleObject::disposing (const css::lang::EventObject& rEvent)
{
    if (rEvent.Source == mxContentWindow)
    {
        mxContentWindow = nullptr;
        mxBorderWindow = nullptr;
    }
    else
    {
        SetWindow(nullptr, nullptr);
    }
}

//----- private ---------------------------------------------------------------

bool AccessibleObject::GetWindowState (const sal_Int64 nType) const
{
    switch (nType)
    {
        case AccessibleStateType::ENABLED:
            return mxContentWindow.is() && mxContentWindow->isEnabled();

        case AccessibleStateType::FOCUSABLE:
            return true;

        case AccessibleStateType::FOCUSED:
            return mbIsFocused;

        case AccessibleStateType::SHOWING:
            return mxContentWindow.is() && mxContentWindow->isVisible();

        default:
            return false;
    }
}

void AccessibleObject::UpdateStateSet()
{
    UpdateState(AccessibleStateType::FOCUSABLE, true);
    UpdateState(AccessibleStateType::VISIBLE, true);
    UpdateState(AccessibleStateType::ENABLED, true);
    UpdateState(AccessibleStateType::MULTI_LINE, true);
    UpdateState(AccessibleStateType::SENSITIVE, true);

    UpdateState(AccessibleStateType::ENABLED, GetWindowState(AccessibleStateType::ENABLED));
    UpdateState(AccessibleStateType::FOCUSED, GetWindowState(AccessibleStateType::FOCUSED));
    UpdateState(AccessibleStateType::SHOWING, GetWindowState(AccessibleStateType::SHOWING));
    //    UpdateState(AccessibleStateType::ACTIVE, GetWindowState(AccessibleStateType::ACTIVE));
}

void AccessibleObject::UpdateState(
    const sal_Int64 nState,
    const bool bValue)
{
    if (((mnStateSet & nState) != 0) == bValue)
        return;
    if (bValue)
    {
        mnStateSet |= nState;
        FireAccessibleEvent(AccessibleEventId::STATE_CHANGED, Any(), Any(nState));
    }
    else
    {
        mnStateSet &= ~nState;
        FireAccessibleEvent(AccessibleEventId::STATE_CHANGED, Any(nState), Any());
    }
}

void AccessibleObject::AddChild (
    const ::rtl::Reference<AccessibleObject>& rpChild)
{
    maChildren.push_back(rpChild);
    rpChild->SetAccessibleParent(this);
    FireAccessibleEvent(AccessibleEventId::INVALIDATE_ALL_CHILDREN, Any(), Any());
}

void AccessibleObject::RemoveChild (
    const ::rtl::Reference<AccessibleObject>& rpChild)
{
    rpChild->SetAccessibleParent(Reference<XAccessible>());
    maChildren.erase(::std::find(maChildren.begin(), maChildren.end(), rpChild));
    FireAccessibleEvent(AccessibleEventId::INVALIDATE_ALL_CHILDREN, Any(), Any());
}

void AccessibleObject::SetIsFocused (const bool bIsFocused)
{
    if (mbIsFocused != bIsFocused)
    {
        mbIsFocused = bIsFocused;
        UpdateStateSet();
    }
}

void AccessibleObject::SetAccessibleName (const OUString& rsName)
{
    if (msName != rsName)
    {
        const OUString sOldName(msName);
        msName = rsName;
        FireAccessibleEvent(AccessibleEventId::NAME_CHANGED, Any(sOldName), Any(msName));
    }
}

void AccessibleObject::FireAccessibleEvent (
    const sal_Int16 nEventId,
    const uno::Any& rOldValue,
    const uno::Any& rNewValue )
{
    NotifyAccessibleEvent(nEventId, rOldValue, rNewValue);
}

awt::Point AccessibleObject::GetRelativeLocation()
{
    awt::Point aLocation;
    if (mxContentWindow.is())
    {
        const awt::Rectangle aContentBox (mxContentWindow->getPosSize());
        aLocation.X = aContentBox.X;
        aLocation.Y = aContentBox.Y;
        if (mxBorderWindow.is())
        {
            const awt::Rectangle aBorderBox (mxBorderWindow->getPosSize());
            aLocation.X += aBorderBox.X;
            aLocation.Y += aBorderBox.Y;
        }
    }
    return aLocation;
}

awt::Size AccessibleObject::GetSize()
{
    if (mxContentWindow.is())
    {
        const awt::Rectangle aBox (mxContentWindow->getPosSize());
        return awt::Size(aBox.Width, aBox.Height);
    }
    else
        return awt::Size();
}

void AccessibleObject::ThrowIfDisposed() const
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        throw lang::DisposedException(u"object has already been disposed"_ustr, uno::Reference<uno::XInterface>(const_cast<uno::XWeak*>(static_cast<uno::XWeak const *>(this))));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
