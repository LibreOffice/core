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

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <cppuhelper/implbase.hxx>
#include <vcl/accessibility/vclxaccessiblecomponent.hxx>
#include <vcl/vclevent.hxx>

// class AccessibleListBox -----------------------------------------------

class SvtIconChoiceCtrl;

class AccessibleIconChoiceCtrl final :
    public cppu::ImplInheritanceHelper<
        VCLXAccessibleComponent,
        css::accessibility::XAccessible,
        css::accessibility::XAccessibleSelection>
{
    css::uno::Reference< css::accessibility::XAccessible > m_xParent;

    virtual ~AccessibleIconChoiceCtrl() override = default;

    /** this function is called upon disposing the component */
    virtual void SAL_CALL   disposing() override;

    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual void    FillAccessibleStateSet( sal_Int64& rStateSet ) override;

    VclPtr< SvtIconChoiceCtrl > getCtrl() const;

public:
    /** OAccessibleBase needs a valid view
        @param  _rIconCtrl
            is the box for which we implement an accessible object
        @param  _xParent
            is our parent accessible object
    */
    AccessibleIconChoiceCtrl(
        SvtIconChoiceCtrl& _rIconCtrl,
        const css::uno::Reference<css::accessibility::XAccessible>& _xParent);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    // XAccessibleContext
    virtual sal_Int64 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;

    // XAccessibleSelection
    void SAL_CALL selectAccessibleChild( sal_Int64 nChildIndex ) override;
    sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int64 nChildIndex ) override;
    void SAL_CALL clearAccessibleSelection(  ) override;
    void SAL_CALL selectAllAccessibleChildren(  ) override;
    sal_Int64 SAL_CALL getSelectedAccessibleChildCount(  ) override;
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
    void SAL_CALL deselectAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
