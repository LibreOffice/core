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

#pragma once

#include <sal/config.h>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>

#include <comphelper/accessibleeventnotifier.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <tools/gen.hxx>

#include <vector>

class SalInstanceIconView;
class SalInstanceIconViewItemAccessible;

using SalInstanceIconViewAccessible_BASE = cppu::WeakImplHelper<
    css::accessibility::XAccessible, css::accessibility::XAccessibleComponent,
    css::accessibility::XAccessibleContext, css::accessibility::XAccessibleSelection,
    css::accessibility::XAccessibleEventBroadcaster>;

class SalInstanceIconViewAccessible final : public SalInstanceIconViewAccessible_BASE
{
public:
    SalInstanceIconViewAccessible(SalInstanceIconView& rControl);

    void ReleaseAllItems();
    void AddAllItems();

    // XAccessible
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
        SAL_CALL getAccessibleContext() override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint(const css::awt::Point& aPoint) override;
    virtual css::awt::Point SAL_CALL getLocation() override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen() override;
    virtual css::awt::Size SAL_CALL getSize() override;
    virtual css::awt::Rectangle SAL_CALL getBounds() override;
    virtual css::uno::Reference<css::accessibility::XAccessible>
        SAL_CALL getAccessibleAtPoint(const css::awt::Point& rPoint) override;
    virtual void grabFocus() override;
    virtual sal_Int32 getForeground() override;
    virtual sal_Int32 getBackground() override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() override;
    virtual css::uno::Reference<css::accessibility::XAccessible>
        SAL_CALL getAccessibleChild(sal_Int32 i) override;
    virtual css::uno::Reference<css::accessibility::XAccessible>
        SAL_CALL getAccessibleParent() override;
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;
    virtual OUString SAL_CALL getAccessibleDescription() override;
    virtual OUString SAL_CALL getAccessibleName() override;
    virtual css::uno::Reference<css::accessibility::XAccessibleRelationSet>
        SAL_CALL getAccessibleRelationSet() override;
    virtual css::uno::Reference<css::accessibility::XAccessibleStateSet>
        SAL_CALL getAccessibleStateSet() override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() override;
    virtual css::lang::Locale SAL_CALL getLocale() override;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild(sal_Int32 nChildIndex) override;
    virtual sal_Bool SAL_CALL isAccessibleChildSelected(sal_Int32 nChildIndex) override;
    virtual void SAL_CALL clearAccessibleSelection() override;
    virtual void SAL_CALL selectAllAccessibleChildren() override;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount() override;
    virtual css::uno::Reference<css::accessibility::XAccessible>
        SAL_CALL getSelectedAccessibleChild(sal_Int32 nSelectedChildIndex) override;
    virtual void SAL_CALL deselectAccessibleChild(sal_Int32 nChildIndex) override;

    // XAccessibleEventBroadcaster
    void SAL_CALL addAccessibleEventListener(
        const css::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener);
    void SAL_CALL removeAccessibleEventListener(
        const css::uno::Reference<css::accessibility::XAccessibleEventListener>& xListener);

private:
    std::vector<rtl::Reference<SalInstanceIconViewItemAccessible>> m_aAccessibleChildren;
    SalInstanceIconView* m_pControl;
    comphelper::AccessibleEventNotifier::TClientId m_nClientId = 0;

    Size GetSizePixel();
    css::uno::Reference<css::accessibility::XAccessibleContext> GetAccessibleParentContext();
    SalInstanceIconViewItemAccessible* GetAccessibleChild(sal_Int32 c);

    void ThrowIfIllegalState();
    void ThrowIfIllegalIndex(sal_Int32 c);
    void NotifyAccessibleEvent(const sal_Int16 _nEventId, const css::uno::Any& _rOldValue,
                               const css::uno::Any& _rNewValue);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
