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
#ifndef INCLUDED_STARMATH_INC_ACCESSIBLESMELEMENT_HXX
#define INCLUDED_STARMATH_INC_ACCESSIBLESMELEMENT_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase3.hxx>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <sal/types.h>
#include <vcl/vclptr.hxx>

class SmElementsControl;

typedef ::cppu::ImplHelper3<css::lang::XServiceInfo, css::accessibility::XAccessible,
                            css::accessibility::XAccessibleAction>
    AccessibleSmElement_BASE;

class AccessibleSmElement final : public comphelper::OAccessibleComponentHelper,
                                  public AccessibleSmElement_BASE
{
    VclPtr<SmElementsControl> m_pSmElementsControl;
    const sal_Int32 m_nIndexInParent; ///< index in the parent XAccessible
    const sal_uInt16 m_nItemId; ///< index in the SmElementsControl
    bool m_bHasFocus;
    sal_Int16 m_nRole;

    ~AccessibleSmElement() override;
    void disposing() override;
    css::awt::Rectangle implGetBounds() override;

    void testAction(sal_Int32) const;

public:
    explicit AccessibleSmElement(SmElementsControl* pSmElementsControl, sal_uInt16 nItemId,
                                 sal_Int32 nIndexInParent);

    void SetFocus(bool _bFocus);
    bool HasFocus() const { return m_bHasFocus; }
    void ReleaseSmElementsControl() { m_pSmElementsControl = nullptr; }
    sal_uInt16 itemId() const { return m_nItemId; }

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    // XServiceInfo
    OUString getImplementationName() override;
    sal_Bool supportsService(const OUString& rServiceName) override;
    css::uno::Sequence<OUString> getSupportedServiceNames() override;

    // XAccessible
    css::uno::Reference<css::accessibility::XAccessibleContext> getAccessibleContext() override;

    // XAccessibleContext
    sal_Int32 getAccessibleChildCount() override;
    css::uno::Reference<css::accessibility::XAccessible> getAccessibleChild(sal_Int32 i) override;
    css::uno::Reference<css::accessibility::XAccessible> getAccessibleParent() override;
    sal_Int32 getAccessibleIndexInParent() override;
    sal_Int16 getAccessibleRole() override;
    OUString getAccessibleDescription() override;
    OUString getAccessibleName() override;
    css::uno::Reference<css::accessibility::XAccessibleRelationSet>
    getAccessibleRelationSet() override;
    css::uno::Reference<css::accessibility::XAccessibleStateSet> getAccessibleStateSet() override;

    // XAccessibleComponent
    css::uno::Reference<css::accessibility::XAccessible>
    getAccessibleAtPoint(const css::awt::Point& aPoint) override;
    void grabFocus() override;
    sal_Int32 getForeground() override;
    sal_Int32 getBackground() override;

    // XAccessibleAction
    sal_Int32 getAccessibleActionCount() override;
    sal_Bool doAccessibleAction(sal_Int32 nIndex) override;
    OUString getAccessibleActionDescription(sal_Int32 nIndex) override;
    css::uno::Reference<css::accessibility::XAccessibleKeyBinding>
    getAccessibleActionKeyBinding(sal_Int32 nIndex) override;
};

#endif // INCLUDED_STARMATH_INC_ACCESSIBLESMELEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
