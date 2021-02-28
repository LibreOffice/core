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

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase3.hxx>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <sal/types.h>

class SmElementsControl;

typedef ::cppu::ImplHelper3<css::lang::XServiceInfo, css::accessibility::XAccessible,
                            css::accessibility::XAccessibleAction>
    AccessibleSmElement_BASE;

class AccessibleSmElement final : public comphelper::OAccessibleComponentHelper,
                                  public AccessibleSmElement_BASE
{
    SmElementsControl* m_pSmElementsControl;
    const sal_Int32 m_nIndexInParent; ///< index in the parent XAccessible
    const sal_uInt16 m_nItemId; ///< index in the SmElementsControl
    bool m_bHasFocus;
    sal_Int16 m_nRole;

    ~AccessibleSmElement() override;
    void SAL_CALL disposing() override;
    css::awt::Rectangle implGetBounds() override;

    void testAction(sal_Int32) const;

public:
    explicit AccessibleSmElement(SmElementsControl* pSmElementsControl, sal_uInt16 nItemId,
                                 sal_Int32 nIndexInParent);

    void SetFocus(bool _bFocus);
    sal_uInt16 itemId() const { return m_nItemId; }

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    // XAccessible
    css::uno::Reference<css::accessibility::XAccessibleContext>
        SAL_CALL getAccessibleContext() override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XAccessibleContext
    sal_Int32 SAL_CALL getAccessibleChildCount() override;
    css::uno::Reference<css::accessibility::XAccessible>
        SAL_CALL getAccessibleChild(sal_Int32 i) override;
    css::uno::Reference<css::accessibility::XAccessible> SAL_CALL getAccessibleParent() override;
    sal_Int32 SAL_CALL getAccessibleIndexInParent() override;
    sal_Int16 SAL_CALL getAccessibleRole() override;
    OUString SAL_CALL getAccessibleDescription() override;
    OUString SAL_CALL getAccessibleName() override;
    css::uno::Reference<css::accessibility::XAccessibleRelationSet>
        SAL_CALL getAccessibleRelationSet() override;
    css::uno::Reference<css::accessibility::XAccessibleStateSet>
        SAL_CALL getAccessibleStateSet() override;

    // XAccessibleComponent
    css::uno::Reference<css::accessibility::XAccessible>
        SAL_CALL getAccessibleAtPoint(const css::awt::Point& aPoint) override;
    void SAL_CALL grabFocus() override;
    sal_Int32 SAL_CALL getForeground() override;
    sal_Int32 SAL_CALL getBackground() override;

    // XAccessibleAction
    sal_Int32 SAL_CALL getAccessibleActionCount() override;
    sal_Bool SAL_CALL doAccessibleAction(sal_Int32 nIndex) override;
    OUString SAL_CALL getAccessibleActionDescription(sal_Int32 nIndex) override;
    css::uno::Reference<css::accessibility::XAccessibleKeyBinding>
        SAL_CALL getAccessibleActionKeyBinding(sal_Int32 nIndex) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
