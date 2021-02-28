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

#pragma once

#include <comphelper/accessiblecomponenthelper.hxx>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase3.hxx>
#include <rtl/ref.hxx>

#include <vector>

class AccessibleSmElement;
class SmElementsControl;

typedef ::cppu::ImplHelper3<css::lang::XServiceInfo, css::accessibility::XAccessible,
                            css::accessibility::XAccessibleSelection>
    AccessibleSmElementsControl_BASE;

class AccessibleSmElementsControl final : public comphelper::OAccessibleComponentHelper,
                                          public AccessibleSmElementsControl_BASE
{
    std::vector<rtl::Reference<AccessibleSmElement>> m_aAccessibleChildren;
    SmElementsControl* m_pControl;

    void UpdateFocus(sal_uInt16);
    inline void TestControl();

    ~AccessibleSmElementsControl() override;
    void SAL_CALL disposing() override;
    css::awt::Rectangle implGetBounds() override;

public:
    AccessibleSmElementsControl(SmElementsControl& rControl);

    void ReleaseAllItems();
    void AddAllItems();
    inline void AcquireFocus();
    inline void ReleaseFocus(sal_uInt16);

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    // XAccessible
    css::uno::Reference<css::accessibility::XAccessibleContext>
        SAL_CALL getAccessibleContext() override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XAccessibleComponent
    sal_Bool SAL_CALL containsPoint(const css::awt::Point& aPoint) override;
    css::uno::Reference<css::accessibility::XAccessible>
        SAL_CALL getAccessibleAtPoint(const css::awt::Point& aPoint) override;
    void SAL_CALL grabFocus() override;
    sal_Int32 SAL_CALL getForeground() override;
    sal_Int32 SAL_CALL getBackground() override;

    // XAccessibleContext
    sal_Int32 SAL_CALL getAccessibleChildCount() override;
    css::uno::Reference<css::accessibility::XAccessible>
        SAL_CALL getAccessibleChild(sal_Int32 i) override;
    css::uno::Reference<css::accessibility::XAccessible> SAL_CALL getAccessibleParent() override;
    sal_Int16 SAL_CALL getAccessibleRole() override;
    OUString SAL_CALL getAccessibleDescription() override;
    OUString SAL_CALL getAccessibleName() override;
    css::uno::Reference<css::accessibility::XAccessibleRelationSet>
        SAL_CALL getAccessibleRelationSet() override;
    css::uno::Reference<css::accessibility::XAccessibleStateSet>
        SAL_CALL getAccessibleStateSet() override;

    // XAccessibleSelection
    void SAL_CALL selectAccessibleChild(sal_Int32 nChildIndex) override;
    sal_Bool SAL_CALL isAccessibleChildSelected(sal_Int32 nChildIndex) override;
    void SAL_CALL clearAccessibleSelection() override;
    void SAL_CALL selectAllAccessibleChildren() override;
    sal_Int32 SAL_CALL getSelectedAccessibleChildCount() override;
    css::uno::Reference<css::accessibility::XAccessible>
        SAL_CALL getSelectedAccessibleChild(sal_Int32 nSelectedChildIndex) override;
    void SAL_CALL deselectAccessibleChild(sal_Int32 nChildIndex) override;
};

void AccessibleSmElementsControl::AcquireFocus() { UpdateFocus(SAL_MAX_UINT16); }

void AccessibleSmElementsControl::ReleaseFocus(sal_uInt16 nPos) { UpdateFocus(nPos); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
