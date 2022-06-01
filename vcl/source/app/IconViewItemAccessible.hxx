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
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>

#include <com/sun/star/awt/Rectangle.hpp>
#include <cppuhelper/implbase.hxx>
#include <tools/gen.hxx>

class SalInstanceIconView;

using AccessibleSmElement_BASE = cppu::WeakImplHelper<
    css::accessibility::XAccessible, css::accessibility::XAccessibleComponent,
    css::accessibility::XAccessibleContext, css::accessibility::XAccessibleAction>;

class SalInstanceIconViewItemAccessible final : public AccessibleSmElement_BASE
{
public:
    explicit SalInstanceIconViewItemAccessible(SalInstanceIconView* pSmElementsControl,
                                               sal_Int32 nIndexInParent);

    void SetFocus(bool _bFocus);
    sal_uInt16 itemIndex() const { return m_nIndex; }

    tools::Rectangle GetBounds();

    // XAccessible
    css::uno::Reference<css::accessibility::XAccessibleContext>
        SAL_CALL getAccessibleContext() override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint(const css::awt::Point& aPoint) override;
    virtual css::awt::Point SAL_CALL getLocation() override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen() override;
    virtual css::awt::Size SAL_CALL getSize() override;
    virtual css::awt::Rectangle SAL_CALL getBounds() override;
    virtual css::uno::Reference<css::accessibility::XAccessible>
        SAL_CALL getAccessibleAtPoint(const css::awt::Point& rPoint) override;
    virtual void SAL_CALL grabFocus() override;
    virtual sal_Int32 SAL_CALL getForeground() override;
    virtual sal_Int32 SAL_CALL getBackground() override;

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

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount() override;
    virtual sal_Bool SAL_CALL doAccessibleAction(sal_Int32 nIndex) override;
    virtual OUString SAL_CALL getAccessibleActionDescription(sal_Int32 nIndex) override;
    virtual css::uno::Reference<css::accessibility::XAccessibleKeyBinding>
        SAL_CALL getAccessibleActionKeyBinding(sal_Int32 nIndex) override;

private:
    SalInstanceIconView* m_pControl;
    const sal_Int32 m_nIndex; ///< index in the parent XAccessible
    bool m_bHasFocus;
    sal_Int16 m_nRole;

    void TestAction(sal_Int32);
    void ThrowIfIllegalState();

    css::uno::Reference<css::accessibility::XAccessibleContext> GetAccessibleParentContext();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
