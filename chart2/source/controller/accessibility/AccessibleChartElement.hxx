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

#include <AccessibleBase.hxx>
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>

namespace com::sun::star::accessibility { class XAccessible; }
namespace com::sun::star::accessibility { class XAccessibleContext; }
namespace com::sun::star::awt { class XFont; }

namespace chart
{

/** Base class for all Chart Accessibility objects except the root node (see AccessibleChartView)

    This class contains a reference to the ChartModel, thus, components can easily access all core functionality.

    Usage Instructions:

    <ul>
     <li>define the getAccessibleName() method of XAccessibleContext</li>
     <li>set the ChartModel using SetChartModel() for the first node before
         creating any children</li>
     <li>override UpdateChildren()</li>
    </ul>
 */

namespace impl
{
typedef ::cppu::ImplInheritanceHelper<
        AccessibleBase,
        css::accessibility::XAccessibleExtendedComponent
        > AccessibleChartElement_Base;
}

class AccessibleChartElement :
    public impl::AccessibleChartElement_Base
{
public:
    AccessibleChartElement( const AccessibleElementInfo & rAccInfo,
                            bool bMayHaveChildren );
    virtual ~AccessibleChartElement() override;

    // ________ AccessibleBase ________
    virtual bool ImplUpdateChildren() override;
    virtual css::uno::Reference< css::accessibility::XAccessible >
        ImplGetAccessibleChildById( sal_Int32 i ) const override;
    virtual sal_Int32 ImplGetAccessibleChildCount() const override;

    // ________ XAccessibleContext ________
    virtual OUString SAL_CALL getAccessibleName() override;
    virtual OUString SAL_CALL getAccessibleDescription() override;

    // ________ XAccessibleExtendedComponent ________
    virtual css::uno::Reference< css::awt::XFont > SAL_CALL getFont() override;
    virtual OUString SAL_CALL getTitledBorderText() override;
    virtual OUString SAL_CALL getToolTipText() override;

    // the following interface is implemented in AccessibleBase, however it is
    // also a (non-virtual) base class of XAccessibleExtendedComponent Thus
    // these methods have to be overridden and forward to AccessibleBase

    // ________ XAccessibleComponent ________
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual css::awt::Rectangle SAL_CALL getBounds() override;
    virtual css::awt::Point SAL_CALL getLocation() override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen() override;
    virtual css::awt::Size SAL_CALL getSize() override;
    virtual void SAL_CALL grabFocus() override;
    virtual sal_Int32 SAL_CALL getForeground() override;
    virtual sal_Int32 SAL_CALL getBackground() override;

    // ________ XServiceInfo ________
    virtual OUString SAL_CALL getImplementationName() override;

private:
    bool                m_bHasText;
    css::uno::Reference< css::accessibility::XAccessibleContext >
                        m_xTextHelper;

    void InitTextEdit();
};

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
