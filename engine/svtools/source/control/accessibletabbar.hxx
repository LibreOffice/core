/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include "accessibletabbarbase.hxx"

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

#include <vector>

namespace accessibility
{

class AccessibleTabBar final
    : public cppu::ImplInheritanceHelper<AccessibleTabBarBase, css::lang::XServiceInfo>
{
    std::vector<rtl::Reference<comphelper::OAccessible>> m_aAccessibleChildren;

    virtual void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    void            FillAccessibleStateSet( sal_Int64& rStateSet );

    // OAccessible
    virtual css::awt::Rectangle implGetBounds(  ) override;

    // XComponent
    virtual void   disposing() override;

public:
    AccessibleTabBar( TabBar* pTabBar );

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& rServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XAccessibleContext
    virtual sal_Int64 getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleParent(  ) override;
    virtual sal_Int64 getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 getAccessibleRole(  ) override;
    virtual OUString getAccessibleDescription(  ) override;
    virtual OUString getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > getAccessibleRelationSet(  ) override;
    virtual sal_Int64 getAccessibleStateSet(  ) override;
    virtual css::lang::Locale getLocale(  ) override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual void grabFocus(  ) override;
    virtual sal_Int32 getForeground(  ) override;
    virtual sal_Int32 getBackground(  ) override;

    // XAccessibleExtendedComponent
    virtual OUString getTitledBorderText(  ) override;
    virtual OUString getToolTipText(  ) override;
};


}   // namespace accessibility



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
