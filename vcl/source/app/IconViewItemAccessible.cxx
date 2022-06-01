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
    //    NotifyAccessibleEvent(css::accessibility::AccessibleEventId::STATE_CHANGED, aOldValue,
    //                          aNewValue);
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

// XAccessible

css::uno::Reference<css::accessibility::XAccessibleContext>
SalInstanceIconViewItemAccessible::getAccessibleContext()
{
    return this;
}

// XAccessibleComponent
sal_Bool SalInstanceIconViewItemAccessible::containsPoint(const css::awt::Point& aPoint)
{
    ThrowIfIllegalState();
    return GetBounds().Contains(VCLPoint(aPoint));
}

css::awt::Point SalInstanceIconViewItemAccessible::getLocation()
{
    ThrowIfIllegalState();
    return AWTPoint(GetBounds().TopLeft());
}

css::awt::Point SalInstanceIconViewItemAccessible::getLocationOnScreen()
{
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
    ThrowIfIllegalState();
    return AWTSize(GetBounds().GetSize());
}

css::awt::Rectangle SalInstanceIconViewItemAccessible::getBounds()
{
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
    ThrowIfIllegalState();
    if (css::uno::Reference<css::accessibility::XAccessibleSelection> rxAccessibleSelection{
            GetAccessibleParentContext(), css::uno::UNO_QUERY })
    {
        rxAccessibleSelection->selectAccessibleChild(getAccessibleIndexInParent());
    }
}

sal_Int32 SalInstanceIconViewItemAccessible::getForeground()
{
    ThrowIfIllegalState();
    return static_cast<sal_Int32>(m_pControl->getWidget()->GetTextColor());
}

sal_Int32 SalInstanceIconViewItemAccessible::getBackground()
{
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
    css::uno::Reference<css::accessibility::XAccessible> xParent;
    if (m_pControl)
        xParent = m_pControl->get_accessible();
    return xParent;
}

sal_Int32 SalInstanceIconViewItemAccessible::getAccessibleIndexInParent() { return m_nIndex; }

sal_Int16 SalInstanceIconViewItemAccessible::getAccessibleRole() { return m_nRole; }

OUString SalInstanceIconViewItemAccessible::getAccessibleDescription()
{
    return getAccessibleName();
}

OUString SalInstanceIconViewItemAccessible::getAccessibleName()
{
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
    // only one action -> "Press"
    return m_pControl->item_is_separator(m_nIndex) ? 0 : 1;
}

void SalInstanceIconViewItemAccessible::TestAction(sal_Int32 nIndex)
{
    ThrowIfIllegalState();
    if (m_pControl->item_is_separator(m_nIndex) || (nIndex != 0))
        throw css::lang::IndexOutOfBoundsException();
}

sal_Bool SalInstanceIconViewItemAccessible::doAccessibleAction(sal_Int32 nIndex)
{
    TestAction(nIndex);

    return m_pControl->item_activate(m_nIndex);
}

OUString SalInstanceIconViewItemAccessible::getAccessibleActionDescription(sal_Int32 nIndex)
{
    TestAction(nIndex);

    return "press";
}

css::uno::Reference<css::accessibility::XAccessibleKeyBinding>
SalInstanceIconViewItemAccessible::getAccessibleActionKeyBinding(sal_Int32 nIndex)
{
    TestAction(nIndex);

    return {};
}

css::lang::Locale SalInstanceIconViewItemAccessible::getLocale()
{
    // simply ask the parent
    if (auto xParent = getAccessibleParent())
        if (auto xParentContext = xParent->getAccessibleContext())
            return xParentContext->getLocale();
    throw css::accessibility::IllegalAccessibleComponentStateException({}, *this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
