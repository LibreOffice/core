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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_ACCESSIBILITY_ACCESSIBLECHARTSHAPE_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_ACCESSIBILITY_ACCESSIBLECHARTSHAPE_HXX

#include "AccessibleBase.hxx"

#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#include <cppuhelper/implbase.hxx>
#include <svx/AccessibleShapeTreeInfo.hxx>

namespace accessibility
{
class AccessibleShape;
}

namespace chart
{

namespace impl
{
typedef ::cppu::ImplInheritanceHelper<
    AccessibleBase,
    css::accessibility::XAccessibleExtendedComponent > AccessibleChartShape_Base;
}

class AccessibleChartShape :
    public impl::AccessibleChartShape_Base
{
public:
    explicit AccessibleChartShape( const AccessibleElementInfo& rAccInfo );
    virtual ~AccessibleChartShape();

    // ________ XServiceInfo ________
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    // ________ XAccessibleContext ________
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleChild( sal_Int32 i )
        throw (css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleDescription()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName()
        throw (css::uno::RuntimeException, std::exception) override;

    // ________ XAccessibleComponent ________
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL getBounds() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocation() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL grabFocus() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getForeground() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground() throw (css::uno::RuntimeException, std::exception) override;

    // ________ XAccessibleExtendedComponent ________
    virtual css::uno::Reference< css::awt::XFont > SAL_CALL getFont()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTitledBorderText()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getToolTipText()
        throw (css::uno::RuntimeException, std::exception) override;

private:
    rtl::Reference<accessibility::AccessibleShape> m_pAccShape;
    ::accessibility::AccessibleShapeTreeInfo m_aShapeTreeInfo;
};

}  // namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
