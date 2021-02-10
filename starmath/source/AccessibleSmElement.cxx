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

#include <AccessibleSmElement.hxx>
#include <ElementsDockingWindow.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <toolkit/helper/convert.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <unotools/accessiblestatesethelper.hxx>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star;
using OContextEntryGuard = ::comphelper::OContextEntryGuard;
using OExternalLockGuard = ::comphelper::OExternalLockGuard;

AccessibleSmElement::AccessibleSmElement(SmElementsControl* pSmElementsControl, sal_uInt16 nItemId,
                                         sal_Int32 nIndexInParent)
    : m_pSmElementsControl(pSmElementsControl)
    , m_nIndexInParent(nIndexInParent)
    , m_nItemId(nItemId)
    , m_bHasFocus(false)
{
    assert(m_pSmElementsControl);
    m_nRole = m_pSmElementsControl->itemIsSeparator(m_nItemId) ? AccessibleRole::SEPARATOR
                                                               : AccessibleRole::PUSH_BUTTON;
}

AccessibleSmElement::~AccessibleSmElement() {}

void AccessibleSmElement::SetFocus(bool bFocus)
{
    if (m_bHasFocus == bFocus)
        return;

    uno::Any aOldValue;
    uno::Any aNewValue;
    if (m_bHasFocus)
        aOldValue <<= AccessibleStateType::FOCUSED;
    else
        aNewValue <<= AccessibleStateType::FOCUSED;
    m_bHasFocus = bFocus;
    NotifyAccessibleEvent(AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue);
}

awt::Rectangle AccessibleSmElement::implGetBounds()
{
    awt::Rectangle aRect;
    if (m_pSmElementsControl)
        aRect = AWTRectangle(m_pSmElementsControl->itemPosRect(m_nItemId));
    return aRect;
}

// XInterface

IMPLEMENT_FORWARD_REFCOUNT(AccessibleSmElement, comphelper::OAccessibleComponentHelper)

uno::Any AccessibleSmElement::queryInterface(const uno::Type& _rType)
{
    if (_rType == cppu::UnoType<XAccessibleAction>::get()
        && (!m_pSmElementsControl || m_pSmElementsControl->itemIsSeparator(m_nItemId)))
        return uno::Any();

    uno::Any aReturn = comphelper::OAccessibleComponentHelper::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = AccessibleSmElement_BASE::queryInterface(_rType);
    return aReturn;
}

// XTypeProvider

IMPLEMENT_FORWARD_XTYPEPROVIDER2(AccessibleSmElement, comphelper::OAccessibleComponentHelper,
                                 AccessibleSmElement_BASE)

// XComponent

void AccessibleSmElement::disposing()
{
    comphelper::OAccessibleComponentHelper::disposing();
    m_pSmElementsControl = nullptr;
}

// XServiceInfo

OUString AccessibleSmElement::getImplementationName()
{
    return "com.sun.star.comp.toolkit.AccessibleSmElement";
}

sal_Bool AccessibleSmElement::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> AccessibleSmElement::getSupportedServiceNames()
{
    return { "com.sun.star.accessibility.AccessibleContext",
             "com.sun.star.accessibility.AccessibleComponent",
             "com.sun.star.accessibility.AccessibleSmElement" };
}

// XAccessible

uno::Reference<XAccessibleContext> AccessibleSmElement::getAccessibleContext() { return this; }

// XAccessibleContext

sal_Int32 AccessibleSmElement::getAccessibleChildCount() { return 0; }

uno::Reference<accessibility::XAccessible> AccessibleSmElement::getAccessibleChild(sal_Int32)
{
    return uno::Reference<XAccessible>();
}

uno::Reference<XAccessible> AccessibleSmElement::getAccessibleParent()
{
    OContextEntryGuard aGuard(this);
    uno::Reference<XAccessible> xParent;
    if (m_pSmElementsControl)
        xParent = m_pSmElementsControl->GetAccessible();
    return xParent;
}

sal_Int32 AccessibleSmElement::getAccessibleIndexInParent()
{
    OContextEntryGuard aGuard(this);
    return m_nIndexInParent;
}

sal_Int16 AccessibleSmElement::getAccessibleRole()
{
    OContextEntryGuard aGuard(this);
    return m_nRole;
}

OUString AccessibleSmElement::getAccessibleDescription() { return getAccessibleName(); }

OUString AccessibleSmElement::getAccessibleName()
{
    OExternalLockGuard aGuard(this);
    OUString aName;
    if (m_pSmElementsControl)
        aName = m_pSmElementsControl->itemName(m_nItemId);
    return aName;
}

uno::Reference<XAccessibleRelationSet> AccessibleSmElement::getAccessibleRelationSet()
{
    OContextEntryGuard aGuard(this);

    return new utl::AccessibleRelationSetHelper;
}

uno::Reference<XAccessibleStateSet> AccessibleSmElement::getAccessibleStateSet()
{
    OExternalLockGuard aGuard(this);

    rtl::Reference<utl::AccessibleStateSetHelper> pStateSetHelper
        = new utl::AccessibleStateSetHelper;

    if (m_pSmElementsControl && !rBHelper.bDisposed && !rBHelper.bInDispose)
    {
        if (m_pSmElementsControl->itemIsVisible(m_nItemId))
            pStateSetHelper->AddState(AccessibleStateType::VISIBLE);
        if (!m_pSmElementsControl->itemIsSeparator(m_nItemId))
        {
            if (m_pSmElementsControl->IsEnabled())
            {
                pStateSetHelper->AddState(AccessibleStateType::ENABLED);
                pStateSetHelper->AddState(AccessibleStateType::SENSITIVE);
            }
            pStateSetHelper->AddState(AccessibleStateType::FOCUSABLE);
            if (m_bHasFocus)
                pStateSetHelper->AddState(AccessibleStateType::FOCUSED);
        }
    }
    else
        pStateSetHelper->AddState(AccessibleStateType::DEFUNC);

    return pStateSetHelper;
}

// XAccessibleComponent

uno::Reference<XAccessible> AccessibleSmElement::getAccessibleAtPoint(const awt::Point&)
{
    return uno::Reference<XAccessible>();
}

void AccessibleSmElement::grabFocus()
{
    uno::Reference<XAccessible> xParent(getAccessibleParent());

    if (xParent.is())
    {
        uno::Reference<XAccessibleSelection> rxAccessibleSelection(xParent->getAccessibleContext(),
                                                                   uno::UNO_QUERY);
        if (rxAccessibleSelection.is())
            rxAccessibleSelection->selectAccessibleChild(getAccessibleIndexInParent());
    }
}

sal_Int32 AccessibleSmElement::getForeground()
{
    OExternalLockGuard aGuard(this);

    Color nColor = SmElementsControl::GetTextColor();
    return sal_Int32(nColor);
}

sal_Int32 AccessibleSmElement::getBackground()
{
    OExternalLockGuard aGuard(this);

    Color nColor = SmElementsControl::GetControlBackground();
    return sal_Int32(nColor);
}

// XAccessibleAction

sal_Int32 AccessibleSmElement::getAccessibleActionCount()
{
    // only one action -> "Press"
    return m_pSmElementsControl->itemIsSeparator(m_nItemId) ? 0 : 1;
}

void AccessibleSmElement::testAction(sal_Int32 nIndex) const
{
    if (!m_pSmElementsControl || m_pSmElementsControl->itemIsSeparator(m_nItemId) || (nIndex != 0))
        throw lang::IndexOutOfBoundsException();
}

sal_Bool AccessibleSmElement::doAccessibleAction(sal_Int32 nIndex)
{
    OExternalLockGuard aGuard(this);

    testAction(nIndex);

    return m_pSmElementsControl->itemTrigger(m_nItemId);
}

OUString AccessibleSmElement::getAccessibleActionDescription(sal_Int32 nIndex)
{
    OExternalLockGuard aGuard(this);

    testAction(nIndex);

    return "press";
}

uno::Reference<XAccessibleKeyBinding>
AccessibleSmElement::getAccessibleActionKeyBinding(sal_Int32 nIndex)
{
    OContextEntryGuard aGuard(this);

    testAction(nIndex);

    return uno::Reference<XAccessibleKeyBinding>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
