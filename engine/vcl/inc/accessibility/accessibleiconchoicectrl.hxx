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

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <cppuhelper/implbase.hxx>
#include <vcl/accessibility/vclxaccessiblecomponent.hxx>

// class AccessibleListBox -----------------------------------------------

class SvtIconChoiceCtrl;

class AccessibleIconChoiceCtrl final
    : public cppu::ImplInheritanceHelper<VCLXAccessibleComponent,
                                         css::accessibility::XAccessibleSelection>
{
    virtual ~AccessibleIconChoiceCtrl() override = default;

    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual void    FillAccessibleStateSet( sal_Int64& rStateSet ) override;

    VclPtr< SvtIconChoiceCtrl > getCtrl() const;

public:
    /** OAccessibleBase needs a valid view
        @param  _rIconCtrl
            is the box for which we implement an accessible object
    */
    AccessibleIconChoiceCtrl(SvtIconChoiceCtrl& _rIconCtrl);

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XAccessibleContext
    virtual sal_Int64 getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleChild( sal_Int64 i ) override;
    virtual sal_Int16 getAccessibleRole(  ) override;
    virtual OUString getAccessibleDescription(  ) override;
    virtual OUString getAccessibleName(  ) override;

    // XAccessibleSelection
    void selectAccessibleChild( sal_Int64 nChildIndex ) override;
    bool isAccessibleChildSelected( sal_Int64 nChildIndex ) override;
    void clearAccessibleSelection(  ) override;
    void selectAllAccessibleChildren(  ) override;
    sal_Int64 getSelectedAccessibleChildCount(  ) override;
    css::uno::Reference< css::accessibility::XAccessible > getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
    void deselectAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
