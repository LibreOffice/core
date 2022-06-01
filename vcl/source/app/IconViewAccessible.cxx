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

#include <sal/config.h>

#include "IconViewAccessible.hxx"
#include "IconViewItemAccessible.hxx"

#include <salvtables.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <toolkit/helper/convert.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <unotools/accessiblestatesethelper.hxx>

SalInstanceIconViewAccessible::SalInstanceIconViewAccessible(SalInstanceIconView& rControl)
    : m_pControl(&rControl)
{
}

void SalInstanceIconViewAccessible::ReleaseAllItems()
{
    if (m_aAccessibleChildren.empty())
        return;

    m_aAccessibleChildren.clear();
    NotifyAccessibleEvent(css::accessibility::AccessibleEventId::INVALIDATE_ALL_CHILDREN, {}, {});
}

void SalInstanceIconViewAccessible::AddAllItems()
{
    assert(m_pControl);
    if (!m_pControl)
        return;

    css::uno::Any aNewName(getAccessibleName());
    NotifyAccessibleEvent(css::accessibility::AccessibleEventId::NAME_CHANGED, css::uno::Any(),
                          aNewName);

    // register the new items
    sal_Int32 nCount = getAccessibleChildCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        css::uno::Any aNewValue;
        aNewValue <<= getAccessibleChild(i);
        NotifyAccessibleEvent(css::accessibility::AccessibleEventId::CHILD, css::uno::Any(),
                              aNewValue);
    }
}

void SalInstanceIconViewAccessible::ThrowIfIllegalState()
{
    if (!m_pControl)
        throw css::accessibility::IllegalAccessibleComponentStateException({}, *this);
}

// Also calls ThrowIfIllegalState
void SalInstanceIconViewAccessible::ThrowIfIllegalIndex(sal_Int32 c)
{
    if (c < 0 || c >= getAccessibleChildCount())
        throw css::lang::IndexOutOfBoundsException();
}

void SalInstanceIconViewAccessible::NotifyAccessibleEvent(const sal_Int16 _nEventId,
                                                          const css::uno::Any& _rOldValue,
                                                          const css::uno::Any& _rNewValue)
{
    if (m_nClientId)
        comphelper::AccessibleEventNotifier::addEvent(m_nClientId,
                                                      { *this, _nEventId, _rNewValue, _rOldValue });
}

css::uno::Reference<css::accessibility::XAccessibleContext>
SalInstanceIconViewAccessible::GetAccessibleParentContext()
{
    if (auto xParent = getAccessibleParent())
        return xParent->getAccessibleContext();
    return {};
}

Size SalInstanceIconViewAccessible::GetSizePixel()
{
    assert(m_pControl);
    return m_pControl->getWidget()->GetOutputSizePixel();
}

SalInstanceIconViewItemAccessible* SalInstanceIconViewAccessible::GetAccessibleChild(sal_Int32 c)
{
    rtl::Reference<SalInstanceIconViewItemAccessible>& pChild = m_aAccessibleChildren[c];
    if (!pChild)
    {
        int nSelectedItemId = m_pControl->get_selected_pos();
        pChild = new SalInstanceIconViewItemAccessible(m_pControl, c);
        if (pChild->itemIndex() == nSelectedItemId)
            pChild->SetFocus(true);
    }
    return pChild.get();
}

// XAccessible
css::uno::Reference<css::accessibility::XAccessibleContext>
SalInstanceIconViewAccessible::getAccessibleContext()
{
    SolarMutexGuard g;
    return this;
}

// XAccessibleComponent
sal_Bool SalInstanceIconViewAccessible::containsPoint(const css::awt::Point& aPoint)
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    return tools::Rectangle({ 0, 0 }, GetSizePixel()).Contains(VCLPoint(aPoint));
}

css::awt::Point SalInstanceIconViewAccessible::getLocation()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    return { 0, 0 };
}

css::awt::Point SalInstanceIconViewAccessible::getLocationOnScreen()
{
    SolarMutexGuard g;
    css::awt::Point aScreenLoc(0, 0);

    if (vcl::Window* pWindow = m_pControl->getWidget())
    {
        tools::Rectangle aRect = pWindow->GetWindowExtentsRelative(nullptr);
        aScreenLoc.X = aRect.Left();
        aScreenLoc.Y = aRect.Top();
    }

    return aScreenLoc;
}

css::awt::Size SalInstanceIconViewAccessible::getSize()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    return AWTSize(GetSizePixel());
}

css::awt::Rectangle SalInstanceIconViewAccessible::getBounds()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    Size s(GetSizePixel());
    return css::awt::Rectangle(0, 0, s.Width(), s.Height());
}

css::uno::Reference<css::accessibility::XAccessible>
SalInstanceIconViewAccessible::getAccessibleAtPoint(const css::awt::Point& rPoint)
{
    SolarMutexGuard g;
    const sal_Int32 count = getAccessibleChildCount();
    for (sal_Int32 i = 0; i < count; ++i)
    {
        auto item = GetAccessibleChild(i);
        if (item->containsPoint(rPoint))
            return item;
    }
    return {};
}

void SalInstanceIconViewAccessible::grabFocus()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    m_pControl->grab_focus();
}

sal_Int32 SalInstanceIconViewAccessible::getForeground()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    return static_cast<sal_Int32>(m_pControl->getWidget()->GetTextColor());
}

sal_Int32 SalInstanceIconViewAccessible::getBackground()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    return static_cast<sal_Int32>(m_pControl->getWidget()->GetBackgroundColor());
}

// XAccessibleContext
sal_Int32 SalInstanceIconViewAccessible::getAccessibleChildCount()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    int nCount = m_pControl->n_children();
    if (m_aAccessibleChildren.size() != static_cast<size_t>(nCount))
        m_aAccessibleChildren.resize(nCount);
    return nCount;
}

css::uno::Reference<css::accessibility::XAccessible>
SalInstanceIconViewAccessible::getAccessibleChild(sal_Int32 c)
{
    SolarMutexGuard g;
    ThrowIfIllegalIndex(c);
    return GetAccessibleChild(c);
}

css::uno::Reference<css::accessibility::XAccessible>
SalInstanceIconViewAccessible::getAccessibleParent()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();

    if (vcl::Window* pParent = m_pControl->getWidget()->GetParent())
        return pParent->GetAccessible();

    return {};
}

sal_Int16 SalInstanceIconViewAccessible::getAccessibleRole()
{
    return css::accessibility::AccessibleRole::SCROLL_PANE;
}

OUString SalInstanceIconViewAccessible::getAccessibleDescription() { return {}; }

OUString SalInstanceIconViewAccessible::getAccessibleName()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    return m_pControl->get_accessible_name();
}

// XAccessibleSelection
void SalInstanceIconViewAccessible::selectAccessibleChild(sal_Int32 nChildIndex)
{
    SolarMutexGuard g;
    ThrowIfIllegalIndex(nChildIndex);
    m_pControl->select(nChildIndex);
}

sal_Bool SalInstanceIconViewAccessible::isAccessibleChildSelected(sal_Int32 nChildIndex)
{
    SolarMutexGuard g;
    ThrowIfIllegalIndex(nChildIndex);
    return (m_pControl->get_selected_pos() == nChildIndex);
}

void SalInstanceIconViewAccessible::clearAccessibleSelection()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    m_pControl->select(-1);
}

void SalInstanceIconViewAccessible::selectAllAccessibleChildren()
{
    // intentionally empty
}

sal_Int32 SalInstanceIconViewAccessible::getSelectedAccessibleChildCount()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    return m_pControl->count_selected_items() > 0 ? 1 : 0;
}

css::uno::Reference<css::accessibility::XAccessible>
    SalInstanceIconViewAccessible::getSelectedAccessibleChild(sal_Int32 /* nSelectedChildIndex */)
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    return getAccessibleChild(m_pControl->get_selected_pos());
}

void SalInstanceIconViewAccessible::deselectAccessibleChild(sal_Int32 nChildIndex)
{
    SolarMutexGuard g;
    ThrowIfIllegalIndex(nChildIndex);
    clearAccessibleSelection(); // there can be just one selected child
}

css::uno::Reference<css::accessibility::XAccessibleRelationSet>
SalInstanceIconViewAccessible::getAccessibleRelationSet()
{
    css::uno::Reference<css::accessibility::XAccessibleRelationSet> xRelSet
        = new utl::AccessibleRelationSetHelper();
    return xRelSet; // empty relation set
}

css::uno::Reference<css::accessibility::XAccessibleStateSet>
SalInstanceIconViewAccessible::getAccessibleStateSet()
{
    SolarMutexGuard g;
    rtl::Reference<utl::AccessibleStateSetHelper> pStateSet = new utl::AccessibleStateSetHelper;

    if (!m_pControl)
        pStateSet->AddState(css::accessibility::AccessibleStateType::DEFUNC);
    else
    {
        pStateSet->AddState(css::accessibility::AccessibleStateType::ENABLED);
        pStateSet->AddState(css::accessibility::AccessibleStateType::FOCUSABLE);
        if (m_pControl->has_focus())
            pStateSet->AddState(css::accessibility::AccessibleStateType::FOCUSED);
        if (m_pControl->is_active())
            pStateSet->AddState(css::accessibility::AccessibleStateType::ACTIVE);
        if (m_pControl->get_visible())
            pStateSet->AddState(css::accessibility::AccessibleStateType::SHOWING);
        if (m_pControl->is_visible())
            pStateSet->AddState(css::accessibility::AccessibleStateType::VISIBLE);
        if (!m_pControl->getWidget()->GetBackgroundColor().IsFullyTransparent())
            pStateSet->AddState(css::accessibility::AccessibleStateType::OPAQUE);
    }

    return pStateSet;
}

sal_Int32 SalInstanceIconViewAccessible::getAccessibleIndexInParent()
{
    SolarMutexGuard g;
    // -1 for child not found/no parent (according to specification)
    sal_Int32 nRet = -1;

    try
    {
        if (auto xParentContext = GetAccessibleParentContext())
        {
            //  iterate over parent's children and search for this object
            // our own XAccessible for comparing with the children of our parent
            css::uno::Reference<css::accessibility::XAccessible> xThis(this);
            sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
            for (sal_Int32 nChild = 0; (nChild < nChildCount) && (-1 == nRet); ++nChild)
            {
                auto xChild(xParentContext->getAccessibleChild(nChild));
                if (xChild == xThis)
                    nRet = nChild;
            }
        }
    }
    catch (const css::uno::Exception&)
    {
        // do nothing
    }

    return nRet;
}

css::lang::Locale SalInstanceIconViewAccessible::getLocale()
{
    SolarMutexGuard g;
    // simply ask the parent
    if (auto xParentContext = GetAccessibleParentContext())
        return xParentContext->getLocale();
    throw css::accessibility::IllegalAccessibleComponentStateException({}, *this);
}

// XAccessibleEventBroadcaster
void SalInstanceIconViewAccessible::addAccessibleEventListener(
    const css::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener)
{
    if (!xListener)
        return;

    SolarMutexGuard g;
    if (!m_pControl)
    {
        xListener->disposing(css::lang::EventObject(*this));
        return;
    }

    if (!m_nClientId)
        m_nClientId = comphelper::AccessibleEventNotifier::registerClient();

    comphelper::AccessibleEventNotifier::addEventListener(m_nClientId, xListener);
}

void SAL_CALL SalInstanceIconViewAccessible::removeAccessibleEventListener(
    const css::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener)
{
    if (!xListener)
        return;

    SolarMutexGuard g;
    // the most recent specification for XComponent states that we should silently ignore
    // the call if we're not alive anymore
    if (!m_pControl)
        return;

    if (!m_nClientId)
        return;

    sal_Int32 nListenerCount
        = comphelper::AccessibleEventNotifier::removeEventListener(m_nClientId, xListener);
    if (!nListenerCount)
    {
        // no listeners anymore
        // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
        // and at least to us not firing any events anymore, in case somebody calls
        // NotifyAccessibleEvent, again
        comphelper::AccessibleEventNotifier::revokeClient(m_nClientId);
        m_nClientId = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
