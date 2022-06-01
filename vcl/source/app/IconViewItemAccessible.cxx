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

#include "IconViewItemAccessible.hxx"

#include <salvtables.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <toolkit/helper/convert.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <unotools/accessiblestatesethelper.hxx>

SalInstanceIconViewItemAccessible::SalInstanceIconViewItemAccessible(
    SalInstanceIconView* pSmElementsControl, sal_Int32 nIndexInParent)
    : m_pControl(pSmElementsControl)
    , m_nIndex(nIndexInParent)
    , m_bHasFocus(false)
{
    assert(m_pControl);
    m_nRole = m_pControl->item_is_separator(m_nIndex)
                  ? css::accessibility::AccessibleRole::SEPARATOR
                  : css::accessibility::AccessibleRole::PUSH_BUTTON;
}

void SalInstanceIconViewItemAccessible::ThrowIfIllegalState()
{
    if (!m_pControl)
        throw css::accessibility::IllegalAccessibleComponentStateException({}, *this);
}

void SalInstanceIconViewItemAccessible::NotifyAccessibleEvent(const sal_Int16 _nEventId,
                                                              const css::uno::Any& _rOldValue,
                                                              const css::uno::Any& _rNewValue)
{
    if (m_nClientId)
        comphelper::AccessibleEventNotifier::addEvent(m_nClientId,
                                                      { *this, _nEventId, _rNewValue, _rOldValue });
}

void SalInstanceIconViewItemAccessible::SetFocus(bool bFocus)
{
    if (m_bHasFocus == bFocus)
        return;

    css::uno::Any aOldValue;
    css::uno::Any aNewValue;
    if (m_bHasFocus)
        aOldValue <<= css::accessibility::AccessibleStateType::FOCUSED;
    else
        aNewValue <<= css::accessibility::AccessibleStateType::FOCUSED;
    m_bHasFocus = bFocus;
    NotifyAccessibleEvent(css::accessibility::AccessibleEventId::STATE_CHANGED, aOldValue,
                          aNewValue);
}

tools::Rectangle SalInstanceIconViewItemAccessible::GetBounds()
{
    assert(m_pControl);
    return m_pControl->get_item_rect(m_nIndex);
}

css::uno::Reference<css::accessibility::XAccessibleContext>
SalInstanceIconViewItemAccessible::GetAccessibleParentContext()
{
    if (auto xParent = getAccessibleParent())
        return xParent->getAccessibleContext();
    return {};
}

void SalInstanceIconViewItemAccessible::TestAction(sal_Int32 nIndex)
{
    ThrowIfIllegalState();
    if (m_pControl->item_is_separator(m_nIndex) || (nIndex != 0))
        throw css::lang::IndexOutOfBoundsException();
}

// XAccessible
css::uno::Reference<css::accessibility::XAccessibleContext>
SalInstanceIconViewItemAccessible::getAccessibleContext()
{
    SolarMutexGuard g;
    return this;
}

// XAccessibleComponent
sal_Bool SalInstanceIconViewItemAccessible::containsPoint(const css::awt::Point& aPoint)
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    return GetBounds().Contains(VCLPoint(aPoint));
}

css::awt::Point SalInstanceIconViewItemAccessible::getLocation()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    return AWTPoint(GetBounds().TopLeft());
}

css::awt::Point SalInstanceIconViewItemAccessible::getLocationOnScreen()
{
    SolarMutexGuard g;
    css::awt::Point aScreenLoc(0, 0);

    if (css::uno::Reference<css::accessibility::XAccessibleComponent> xParentComponent{
            GetAccessibleParentContext(), css::uno::UNO_QUERY })
    {
        css::awt::Point aParentScreenLoc(xParentComponent->getLocationOnScreen());
        css::awt::Point aOwnRelativeLoc(getLocation());
        aScreenLoc.X = aParentScreenLoc.X + aOwnRelativeLoc.X;
        aScreenLoc.Y = aParentScreenLoc.Y + aOwnRelativeLoc.Y;
    }

    return aScreenLoc;
}

css::awt::Size SalInstanceIconViewItemAccessible::getSize()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    return AWTSize(GetBounds().GetSize());
}

css::awt::Rectangle SalInstanceIconViewItemAccessible::getBounds()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    return AWTRectangle(GetBounds());
}

css::uno::Reference<css::accessibility::XAccessible>
SalInstanceIconViewItemAccessible::getAccessibleAtPoint(const css::awt::Point& /* rPoint */)
{
    return {};
}

void SalInstanceIconViewItemAccessible::grabFocus()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    if (css::uno::Reference<css::accessibility::XAccessibleSelection> rxAccessibleSelection{
            GetAccessibleParentContext(), css::uno::UNO_QUERY })
    {
        rxAccessibleSelection->selectAccessibleChild(getAccessibleIndexInParent());
    }
}

sal_Int32 SalInstanceIconViewItemAccessible::getForeground()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    return static_cast<sal_Int32>(m_pControl->getWidget()->GetTextColor());
}

sal_Int32 SalInstanceIconViewItemAccessible::getBackground()
{
    SolarMutexGuard g;
    ThrowIfIllegalState();
    return static_cast<sal_Int32>(m_pControl->getWidget()->GetBackgroundColor());
}

// XAccessibleContext
sal_Int32 SalInstanceIconViewItemAccessible::getAccessibleChildCount() { return 0; }

css::uno::Reference<css::accessibility::XAccessible>
    SalInstanceIconViewItemAccessible::getAccessibleChild(sal_Int32)
{
    return css::uno::Reference<XAccessible>();
}

css::uno::Reference<css::accessibility::XAccessible>
SalInstanceIconViewItemAccessible::getAccessibleParent()
{
    SolarMutexGuard g;
    css::uno::Reference<css::accessibility::XAccessible> xParent;
    if (m_pControl)
        xParent = m_pControl->get_accessible();
    return xParent;
}

sal_Int32 SalInstanceIconViewItemAccessible::getAccessibleIndexInParent()
{
    SolarMutexGuard g;
    return m_nIndex;
}

sal_Int16 SalInstanceIconViewItemAccessible::getAccessibleRole()
{
    SolarMutexGuard g;
    return m_nRole;
}

OUString SalInstanceIconViewItemAccessible::getAccessibleDescription()
{
    SolarMutexGuard g;
    return getAccessibleName();
}

OUString SalInstanceIconViewItemAccessible::getAccessibleName()
{
    SolarMutexGuard g;
    if (m_pControl)
        return m_pControl->get_item_accessible_text(m_nIndex);
    return {};
}

css::uno::Reference<css::accessibility::XAccessibleRelationSet>
SalInstanceIconViewItemAccessible::getAccessibleRelationSet()
{
    return new utl::AccessibleRelationSetHelper;
}

css::uno::Reference<css::accessibility::XAccessibleStateSet>
SalInstanceIconViewItemAccessible::getAccessibleStateSet()
{
    SolarMutexGuard g;
    rtl::Reference<utl::AccessibleStateSetHelper> pStateSetHelper
        = new utl::AccessibleStateSetHelper;

    if (m_pControl)
    {
        if (m_pControl->item_is_visible(m_nIndex))
            pStateSetHelper->AddState(css::accessibility::AccessibleStateType::VISIBLE);
        if (!m_pControl->item_is_separator(m_nIndex))
        {
            if (m_pControl->get_sensitive())
            {
                pStateSetHelper->AddState(css::accessibility::AccessibleStateType::ENABLED);
                pStateSetHelper->AddState(css::accessibility::AccessibleStateType::SENSITIVE);
            }
            pStateSetHelper->AddState(css::accessibility::AccessibleStateType::FOCUSABLE);
            if (m_bHasFocus)
                pStateSetHelper->AddState(css::accessibility::AccessibleStateType::FOCUSED);
        }
    }
    else
        pStateSetHelper->AddState(css::accessibility::AccessibleStateType::DEFUNC);

    return pStateSetHelper;
}

// XAccessibleAction
sal_Int32 SalInstanceIconViewItemAccessible::getAccessibleActionCount()
{
    SolarMutexGuard g;
    // only one action -> "Press"
    return m_pControl->item_is_separator(m_nIndex) ? 0 : 1;
}

sal_Bool SalInstanceIconViewItemAccessible::doAccessibleAction(sal_Int32 nIndex)
{
    SolarMutexGuard g;
    TestAction(nIndex);

    return m_pControl->item_activate(m_nIndex);
}

OUString SalInstanceIconViewItemAccessible::getAccessibleActionDescription(sal_Int32 nIndex)
{
    SolarMutexGuard g;
    TestAction(nIndex);

    return "press";
}

css::uno::Reference<css::accessibility::XAccessibleKeyBinding>
SalInstanceIconViewItemAccessible::getAccessibleActionKeyBinding(sal_Int32 nIndex)
{
    SolarMutexGuard g;
    TestAction(nIndex);

    return {};
}

css::lang::Locale SalInstanceIconViewItemAccessible::getLocale()
{
    SolarMutexGuard g;
    // simply ask the parent
    if (auto xParent = getAccessibleParent())
        if (auto xParentContext = xParent->getAccessibleContext())
            return xParentContext->getLocale();
    throw css::accessibility::IllegalAccessibleComponentStateException({}, *this);
}

// XAccessibleEventBroadcaster
void SalInstanceIconViewItemAccessible::addAccessibleEventListener(
    const css::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener)
{
    SolarMutexGuard g;
    if (!xListener)
        return;

    ThrowIfIllegalState();
    if (!m_nClientId)
        m_nClientId = comphelper::AccessibleEventNotifier::registerClient();

    comphelper::AccessibleEventNotifier::addEventListener(m_nClientId, xListener);
}

void SAL_CALL SalInstanceIconViewItemAccessible::removeAccessibleEventListener(
    const css::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener)
{
    SolarMutexGuard g;
    if (!xListener)
        return;

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
        comphelper::AccessibleEventNotifier::revokeClient(std::exchange(m_nClientId, 0));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
